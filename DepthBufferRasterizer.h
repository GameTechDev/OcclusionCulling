////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
////////////////////////////////////////////////////////////////////////////////
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