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

#include "AABBoxMaskedRasterizerAVXMT.h"
#include "DepthBufferMaskedRasterizerAVXMT.h"
#include "MaskedOcclusionCulling\MaskedOcclusionCulling.h"
#include "MaskedOcclusionCulling\CullingThreadpool.h"

AABBoxMaskedRasterizerAVXMT::AABBoxMaskedRasterizerAVXMT(MaskedOcclusionCulling *moc, CullingThreadpool *ctp, DepthBufferMaskedRasterizerAVXMT *rasterizer)
	: AABBoxRasterizerAVX()
{
	mMaskedOcclusionCulling = moc;
    mMOCThreadpool = ctp;
    mRasterizer = rasterizer;

    // Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
    _mm_setcsr( _mm_getcsr( ) | 0x8040 );
}

AABBoxMaskedRasterizerAVXMT::~AABBoxMaskedRasterizerAVXMT()
{

}

//------------------------------------------------------------------------------
// For each occludee model
// * Determine if the occludee model AABox is within the viewing frustum 
// * Transform the AABBox to screen space
// * Rasterize the triangles that make up the AABBox
// * Depth test the raterized triangles against the CPU rasterized depth buffer
//-----------------------------------------------------------------------------
void AABBoxMaskedRasterizerAVXMT::TransformAABBoxAndDepthTest(CPUTCamera *pCamera, UINT idx)
{
    {
        mMOCThreadpool->Flush( );
        mMOCThreadpool->SuspendThreads( );
    }

    mTaskData[idx].idx = idx;
    mTaskData[idx].pAABB = this;

    mpCamera[idx] = pCamera;
    gTaskMgr.CreateTaskSet( &AABBoxMaskedRasterizerAVXMT::TransformAABBoxAndDepthTest, &mTaskData[idx], mNumDepthTestTasks, NULL, 0, "TransformAABBoxAndDepthTest", &gAABBoxDepthTest[idx] );
}

void AABBoxMaskedRasterizerAVXMT::WaitForTaskToFinish(UINT idx)
{
    // Wait for the task set
    gTaskMgr.WaitForSet( gAABBoxDepthTest[idx] );
}

void AABBoxMaskedRasterizerAVXMT::ReleaseTaskHandles(UINT idx)
{
    // Release the task set
    if( mEnableFCulling )
    {
        gTaskMgr.ReleaseHandle( gInsideViewFrustum[idx] );
    }
    else
    {
        gTaskMgr.ReleaseHandle( gTooSmall[idx] );
    }
    gTaskMgr.ReleaseHandle( gActiveModels[idx] );
    //gTaskMgr.ReleaseHandle( gXformMesh[idx] );
    //gTaskMgr.ReleaseHandle( gBinMesh[idx] );
    //gTaskMgr.ReleaseHandle( gSortBins[idx] );
    //gTaskMgr.ReleaseHandle( gRasterize[idx] );
    gTaskMgr.ReleaseHandle( gAABBoxDepthTest[idx] );

    gInsideViewFrustum[idx] = gTooSmall[idx] = gActiveModels[idx] = gXformMesh[idx] = gBinMesh[idx] = gSortBins[idx] = gRasterize[idx] = TASKSETHANDLE_INVALID;
    gAABBoxDepthTest[idx] = TASKSETHANDLE_INVALID;

    LARGE_INTEGER startTime = mStartTime[idx][0];
    LARGE_INTEGER stopTime = mStopTime[idx][0];
    for( UINT i = 0; i < mNumDepthTestTasks; i++ )
    {
        startTime = startTime.QuadPart > mStartTime[idx][i].QuadPart ? mStartTime[idx][i] : startTime;
        stopTime = stopTime.QuadPart < mStopTime[idx][i].QuadPart ? mStopTime[idx][i] : stopTime;
    }
    mDepthTestTime[mTimeCounter++] = ( (double)( stopTime.QuadPart - startTime.QuadPart ) ) / ( (double)glFrequency.QuadPart );
    mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;
}

void AABBoxMaskedRasterizerAVXMT::TransformAABBoxAndDepthTest( VOID* pTaskData, INT context, UINT taskId, UINT taskCount )
{
    PerTaskData *pPerTaskData = (PerTaskData*)pTaskData;
    pPerTaskData->pAABB->TransformAABBoxAndDepthTest( taskId, taskCount, pPerTaskData->idx );
}

//--------------------------------------------------------------------------------
// Determine the batch of occludee models each task should work on
// For each occludee model in the batch
// * Transform the AABBox to screen space
// * Rasterize the triangles that make up the AABBox
// * Depth test the raterized triangles against the CPU rasterized depth buffer
//--------------------------------------------------------------------------------
void AABBoxMaskedRasterizerAVXMT::TransformAABBoxAndDepthTest( UINT taskId, UINT taskCount, UINT idx )
{
    QueryPerformanceCounter( &mStartTime[idx][taskId] );

    BoxTestSetupSSE setup;
    setup.Init( mViewMatrix[idx], mProjMatrix[idx], viewportMatrixMaskedOcclusionCulling, mpCamera[idx], mOccludeeSizeThreshold );

    __m128 xformedPos[AABB_VERTICES];
    __m128 cumulativeMatrix[4];

    static const UINT kChunkSize = 64;
    for( UINT base = taskId*kChunkSize; base < mNumModels; base += mNumDepthTestTasks * kChunkSize )
    {
        UINT end = min( base + kChunkSize, mNumModels );
        if( mEnableFCulling )
        {
            CalcInsideFrustum( &mpCamera[idx]->mFrustum, base, end, idx );
        }
        for( UINT i = base; i < end; i++ )
        {
            mpVisible[idx][i] = false;

            if( mpInsideFrustum[idx][i] && !mpTransformedAABBox[i].IsTooSmall( setup, cumulativeMatrix ) )
            {
                // see CullingThreadpool::CullingResult CullingThreadpool::TestRect(float xmin, float ymin, float xmax, float ymax, float wmin) - it just calls MaskedOcclusionCulling::TestRect

                PreTestResult res = mpTransformedAABBox[i].TransformAndPreTestAABBox( xformedPos, cumulativeMatrix, mMaskedOcclusionCulling );
                //if(res == ePT_UNSURE)
                //{
                //	mpVisible[idx][i] = mpTransformedAABBox[i].RasterizeAndDepthTestAABBox(mMaskedOcclusionCulling, xformedPos);
                //}
                //else
                //{
                mpVisible[idx][i] = ( res != ePT_INVISIBLE );
                //}
            }		
        }
    }
    QueryPerformanceCounter( &mStopTime[idx][taskId] );
}
