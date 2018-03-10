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
//--------------------------------------------------------------------------------------
#ifndef DEPTHBUFFERMASKEDRASTERIZERAVXMT_H
#define DEPTHBUFFERMASKEDRASTERIZERAVXMT_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "DepthBufferRasterizerAVX.h"

class MaskedOcclusionCulling;
class CullingThreadpool;

class DepthBufferMaskedRasterizerAVXMT : public DepthBufferRasterizerAVX
{
    private:
        MaskedOcclusionCulling *mMaskedOcclusionCulling;
        CullingThreadpool *mMOCThreadpool;

        std::atomic_bool        mMOCDepthDirty;
        std::atomic_bool        mMOCShutdown;
        std::mutex              mMOCDepthClearLock;
        std::thread             mMOCDepthClearThread;
        std::condition_variable mMOCDepthClearCV;

        UINT                    mNumRasterizedTrisSimple;

	public:
		DepthBufferMaskedRasterizerAVXMT(MaskedOcclusionCulling *moc, CullingThreadpool * ctp);
		~DepthBufferMaskedRasterizerAVXMT();

        struct PerTaskData
        {
            UINT idx;
            DepthBufferMaskedRasterizerAVXMT *pDBR;
        };
        PerTaskData mTaskData[2];

		void TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx);
		void ComputeR2DBTime(UINT idx);

        virtual UINT GetNumRasterizedTriangles( UINT idx ) override { return mNumRasterizedTrisSimple; }

        void MOCDepthSetDirty( );
        void MOCDepthWaitClear( );
        void MOCDepthClearProc( );

	private:
		void ActiveModels(UINT idx);
		void TransformAndRasterizeMeshes(UINT idx);


        static void InsideViewFrustum( VOID* taskData, INT context, UINT taskId, UINT taskCount );
        void InsideViewFrustum( UINT taskId, UINT taskCount, UINT idx );

        static void TooSmall( VOID* taskData, INT context, UINT taskId, UINT taskCount );
        void TooSmall( UINT taskId, UINT taskCount, UINT idx );

        static void ActiveModels( VOID* taskData, INT context, UINT taskId, UINT taskCount );
        void ActiveModels( UINT taskId, UINT idx );
};

#endif  //DEPTHBUFFERMASKEDRASTERIZERAVXST_H