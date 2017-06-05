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

#ifndef AABBOXRASTERIZER_H
#define AABBOXRASTERIZER_H

#include "CPUT_DX11.h"
#include "TaskMgrTBB.h"
#include "Constants.h"

class AABBoxRasterizer
{
	public:
		AABBoxRasterizer();
		virtual ~AABBoxRasterizer();
		virtual void CreateTransformedAABBoxes(CPUTAssetSet **pAssetSet, UINT numAssetSets) = 0;
		virtual void TransformAABBoxAndDepthTest(CPUTCamera *pCamera, UINT idx) = 0;
		virtual void WaitForTaskToFinish(UINT idx) = 0;
		virtual void ReleaseTaskHandles(UINT idx) = 0;
		virtual void RenderVisible(CPUTAssetSet **pAssetSet,
								   CPUTRenderParametersDX &renderParams,
								   UINT numAssetSets,
								   UINT idx) = 0;
		virtual void Render(CPUTAssetSet **pAssetSet,
							CPUTRenderParametersDX &renderParams,
							UINT numAssetSets,
							UINT idx) = 0;

		virtual void ResetInsideFrustum() = 0; 
		virtual void SetViewProjMatrix(float4x4 *viewMatrix, float4x4 *projMatrix, UINT idx) = 0;
		virtual void SetCPURenderTargetPixels(UINT *pRenderTargetPixels, UINT idx) = 0;
		virtual void SetDepthSummaryBuffer(const float *pDepthSummary, UINT idx) = 0;
		virtual void SetDepthTestTasks(UINT numTasks) = 0;
		virtual void SetOccludeeSizeThreshold(float occludeeSizeThreshold) = 0;
		virtual void SetCamera(CPUTCamera *pCamera, UINT idx) = 0;
		virtual void SetEnableFCulling(bool enableFCulling) = 0;

		virtual UINT GetNumOccludees() = 0;
		virtual UINT GetNumCulled(UINT idx) = 0;
		virtual double GetDepthTestTime() = 0;
		virtual UINT GetNumTriangles() = 0;
		virtual UINT GetNumCulledTriangles(UINT idx) = 0;
		virtual UINT GetNumTrisRendered() = 0;
		virtual UINT GetNumFCullCount() = 0;

	protected:
		LARGE_INTEGER mStartTime[2][NUM_DT_TASKS];
		LARGE_INTEGER mStopTime[2][NUM_DT_TASKS];
};

#endif //AABBOXRASTERIZER_H