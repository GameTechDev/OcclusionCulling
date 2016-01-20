//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
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
#ifndef DEPTHBUFFERRASTERIZER_H
#define DEPTHBUFFERRASTERIZER_H

#include "CPUT_DX11.h"
#include "TaskMgrTBB.h"
#include "Constants.h"

class DepthBufferRasterizer
{
	public:
		DepthBufferRasterizer();
		virtual ~DepthBufferRasterizer();
		virtual void CreateTransformedModels(CPUTAssetSet **pAssetSet, UINT numAssetSets) = 0;
		virtual void TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx) = 0;

		virtual void ResetInsideFrustum() = 0;
		virtual void ComputeR2DBTime(UINT idx) = 0;
		virtual void SetEnableFCulling(bool enableFCulling) = 0;
		virtual void SetViewProj(float4x4 *viewMatrix, float4x4 *projMatrix, UINT idx) = 0;
		virtual void SetCPURenderTargetPixels(UINT *pRenderTargetPixels, UINT idx) = 0;
		virtual const float *GetDepthSummaryBuffer(UINT idx) = 0;
		virtual void SetOccluderSizeThreshold(float occluderSizeThreshold) = 0;
		virtual inline void SetCamera(CPUTCamera *pCamera, UINT idx) = 0;

		virtual UINT GetNumOccluders() = 0;
		virtual UINT GetNumOccludersR2DB(UINT idx) = 0;
		virtual double GetRasterizeTime() = 0;
		virtual UINT GetNumTriangles() = 0;
		virtual UINT GetNumRasterizedTriangles(UINT idx) = 0;

	protected:
		LARGE_INTEGER mStartTime[2];
		LARGE_INTEGER mStopTime[2][NUM_TILES];
};

#endif //DEPTHBUFFERRASTERIZER