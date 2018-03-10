//--------------------------------------------------------------------------------------
// Copyright 2015 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//
//--------------------------------------------------------------------------------------
#include "DepthBufferMaskedRasterizerAVXMT.h"
#include "MaskedOcclusionCulling\MaskedOcclusionCulling.h"
#include "MaskedOcclusionCulling\CullingThreadpool.h"

#include"HelperMT.h"

DepthBufferMaskedRasterizerAVXMT::DepthBufferMaskedRasterizerAVXMT(MaskedOcclusionCulling *moc, CullingThreadpool * ctp)
	: DepthBufferRasterizerAVX()
{
	mMaskedOcclusionCulling = moc;
    mMOCThreadpool = ctp;
    mMOCDepthDirty = false;
    mMOCShutdown = false;

	memset(mNumRasterizedTris, 0, sizeof(UINT) * 2 * NUM_TILES);

    mNumRasterizedTrisSimple = 0;

    mMOCDepthClearThread = std::thread( &DepthBufferMaskedRasterizerAVXMT::MOCDepthClearProc, this );

    MOCDepthSetDirty( );
}

DepthBufferMaskedRasterizerAVXMT::~DepthBufferMaskedRasterizerAVXMT()
{
    mMOCShutdown = true;
    mMOCDepthClearCV.notify_one();
    MOCDepthWaitClear( );
    mMOCDepthClearThread.join();
}

void DepthBufferMaskedRasterizerAVXMT::MOCDepthSetDirty( )
{
    // are we clearing? this should never happen - indicates a serious bug
    assert( !mMOCDepthDirty );

    // dispatch clear
    mMOCDepthDirty = true;
    mMOCDepthClearCV.notify_one();
}

void DepthBufferMaskedRasterizerAVXMT::MOCDepthWaitClear( )
{
    std::unique_lock<std::mutex> lock( mMOCDepthClearLock );

    // wait until cleared
    mMOCDepthClearCV.wait( lock, [ &dirty = mMOCDepthDirty ] {return !dirty; } );
}

void DepthBufferMaskedRasterizerAVXMT::MOCDepthClearProc( )
{
    while( true )
    {
        // own the lock (until mMOCDepthClearCV plays with it)
        {
            std::unique_lock<std::mutex> lock( mMOCDepthClearLock );

            // wait until dirty
            mMOCDepthClearCV.wait( lock, [ &dirty = mMOCDepthDirty, &shutdown = mMOCShutdown] { return dirty || shutdown; } );

            if( mMOCShutdown )
                return;

            assert( mMOCDepthDirty );

            {
                mMOCThreadpool->ClearBuffer();
                mMOCDepthDirty = false;
            }
        }
        // Unlocking by leaving the lock scope above is done before notifying, to avoid waking up the waiting thread only to block again
        mMOCDepthClearCV.notify_one();
    }    
}

//------------------------------------------------------------------------------
// * Determine if the occludee model is inside view frustum
// * Transform the occluder models on the CPU
// * Bin the occluder triangles into tiles that the frame buffer is divided into
// * Rasterize the occluder triangles to the CPU depth buffer
//-------------------------------------------------------------------------------
void DepthBufferMaskedRasterizerAVXMT::TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx)
{
    // Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
    _mm_setcsr( _mm_getcsr( ) | 0x8040 );

    // if you want to contain all MOC processing to here
    //MOCDepthSetDirty( );

    // no double-buffering in this implementation!
    assert( idx == 0 );

    QueryPerformanceCounter(&mStartTime[idx]);

    mpCamera[idx] = pCamera;
    mTaskData[idx].idx = idx;
    mTaskData[idx].pDBR = this;

    static const unsigned int kNumOccluderVisTasks = 32;

    {
        if( mEnableFCulling )
        {
            gTaskMgr.CreateTaskSet( &DepthBufferMaskedRasterizerAVXMT::InsideViewFrustum, &mTaskData[idx], kNumOccluderVisTasks, NULL, 0, "Is Visible", &gInsideViewFrustum[idx] );
            gTaskMgr.CreateTaskSet( &DepthBufferMaskedRasterizerAVXMT::ActiveModels, &mTaskData[idx], 1, &gInsideViewFrustum[idx], 1, "IsActive", &gActiveModels[idx] );
        }
        else
        {
            gTaskMgr.CreateTaskSet( &DepthBufferMaskedRasterizerAVXMT::TooSmall, &mTaskData[idx], kNumOccluderVisTasks, NULL, 0, "TooSmall", &gTooSmall[idx] );
            gTaskMgr.CreateTaskSet( &DepthBufferMaskedRasterizerAVXMT::ActiveModels, &mTaskData[idx], 1, &gTooSmall[idx], 1, "IsActive", &gActiveModels[idx] );
        }
    }

    {
        // good time as any to wake them up - while waiting for the above tasks to finish!
        mMOCThreadpool->WakeThreads( );
    }

    {
        gTaskMgr.WaitForSet( gActiveModels[idx] );
        //gTaskMgr.WaitForSet( gXformMesh[idx] );
    }

	TransformAndRasterizeMeshes(idx);

	QueryPerformanceCounter(&mStopTime[idx][0]);
	mRasterizeTime[mTimeCounter++] = ((double)(mStopTime[idx][0].QuadPart - mStartTime[idx].QuadPart)) / ((double)glFrequency.QuadPart);
	mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;
}

void DepthBufferMaskedRasterizerAVXMT::ActiveModels(UINT idx)
{
    // no double-buffering in this implementation!
    assert( idx == 0 );

	ResetActive(idx);
	for (UINT i = 0; i < mNumModels1; i++)
	{
		if(mpTransformedModels1[i].IsRasterized2DB(idx))
		{
			Activate(i, idx);
		}
	}
}

void DepthBufferMaskedRasterizerAVXMT::TransformAndRasterizeMeshes(UINT idx)
{
    // no double-buffering in this implementation!
    assert( idx == 0 );

    // Sort models roughly from front to back
    {
	    std::sort(mpModelIndexA[idx], mpModelIndexA[idx] + mNumModelsA[idx], [=](int a, int b) { return mpTransformedModels1[a].GetDepth() < mpTransformedModels1[b].GetDepth(); });
    }

    // before we can start rendering, need to make sure clear pass is done
    {
        MOCDepthWaitClear( );
    }

    {

        mNumRasterizedTrisSimple = 0;
	    for (UINT active = 0; active < mNumModelsA[idx]; active++)
	    {
		    UINT ss = mpModelIndexA[idx][active];

		    mpTransformedModels1[ss].TransformAndRasterizeTrianglesMT(mMOCThreadpool, idx);
            mNumRasterizedTrisSimple += mpTransformedModels1[ss].GetNumTriangles();
	    }
    }

#if 0 // just do it all one more time
    // Sort models roughly from back to front
    {
        rmt_ScopedCPUSample( SortBackToFront, 0 );
        std::sort( mpModelIndexA[idx], mpModelIndexA[idx] + mNumModelsA[idx], [ = ]( int a, int b ) { return mpTransformedModels1[a].GetDepth( ) > mpTransformedModels1[b].GetDepth( ); } );
    }
    {
        rmt_ScopedCPUSample( MOCRasterizeDispatch, 0 );

        for( UINT active = 0; active < mNumModelsA[idx]; active++ )
        {
            UINT ss = mpModelIndexA[idx][active];
            mpTransformedModels1[ss].TransformAndRasterizeTrianglesMT( mMOCThreadpool, idx );
        }
    }

#endif
}

void DepthBufferMaskedRasterizerAVXMT::ComputeR2DBTime(UINT idx)
{
}

void DepthBufferMaskedRasterizerAVXMT::InsideViewFrustum( VOID* taskData, INT context, UINT taskId, UINT taskCount )
{
    PerTaskData *pTaskData = (PerTaskData*)taskData;
    pTaskData->pDBR->InsideViewFrustum( taskId, taskCount, pTaskData->idx );
}

void DepthBufferMaskedRasterizerAVXMT::InsideViewFrustum( UINT taskId, UINT taskCount, UINT idx )
{
    // no double-buffering in this implementation!
    assert( idx == 0 );

    UINT start, end;
    GetWorkExtent( &start, &end, taskId, taskCount, mNumModels1 );

    BoxTestSetupSSE setup;
    setup.Init( mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrixMaskedOcclusionCulling, mpCamera[idx], mOccluderSizeThreshold );

    for( UINT i = start; i < end; i++ )
    {
        mpTransformedModels1[i].InsideViewFrustum( setup, idx );
    }
}

void DepthBufferMaskedRasterizerAVXMT::TooSmall( VOID* taskData, INT context, UINT taskId, UINT taskCount )
{
    PerTaskData *pTaskData = (PerTaskData*)taskData;
    pTaskData->pDBR->TooSmall( taskId, taskCount, pTaskData->idx );
}

void DepthBufferMaskedRasterizerAVXMT::TooSmall( UINT taskId, UINT taskCount, UINT idx )
{
    // no double-buffering in this implementation!
    assert( idx == 0 );

    UINT start, end;
    GetWorkExtent( &start, &end, taskId, taskCount, mNumModels1 );

    BoxTestSetupSSE setup;
    setup.Init( mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrixMaskedOcclusionCulling, mpCamera[idx], mOccluderSizeThreshold );

    for( UINT i = start; i < end; i++ )
    {
        mpTransformedModels1[i].TooSmall( setup, idx );
    }
}

void DepthBufferMaskedRasterizerAVXMT::ActiveModels( VOID* taskData, INT context, UINT taskId, UINT taskCount )
{
    PerTaskData *pTaskData = (PerTaskData*)taskData;
    pTaskData->pDBR->ActiveModels( taskId, pTaskData->idx );
}

void DepthBufferMaskedRasterizerAVXMT::ActiveModels( UINT taskId, UINT idx )
{
    // no double-buffering in this implementation!
    assert( idx == 0 );

    ResetActive( idx );
    for( UINT i = 0; i < mNumModels1; i++ )
    {
        if( mpTransformedModels1[i].IsRasterized2DB( idx ) )
        {
            Activate( i, idx );
        }
    }

}
