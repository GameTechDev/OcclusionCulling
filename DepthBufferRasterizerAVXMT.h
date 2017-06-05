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
#ifndef DEPTHBUFFERRASTERIZERAVXMT_H
#define DEPTHBUFFERRASTERIZERAVXMT_H

#include "DepthBufferRasterizerAVX.h"

class DepthBufferRasterizerAVXMT : public DepthBufferRasterizerAVX
{
	public:
		DepthBufferRasterizerAVXMT();
		~DepthBufferRasterizerAVXMT();

		struct PerTaskData
		{
			UINT idx;
			DepthBufferRasterizerAVXMT *pDBR; 
		};

		PerTaskData mTaskData[2];
		void TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx);
		void ComputeR2DBTime(UINT idx);

	private:
		static void InsideViewFrustum(VOID* taskData, INT context, UINT taskId, UINT taskCount);
		void InsideViewFrustum(UINT taskId, UINT taskCount, UINT idx);

		static void TooSmall(VOID* taskData, INT context, UINT taskId, UINT taskCount);
		void TooSmall(UINT taskId, UINT taskCount, UINT idx);

		static void ActiveModels(VOID* taskData, INT context, UINT taskId, UINT taskCount);
		void ActiveModels(UINT taskId, UINT idx);

		static void TransformMeshes(VOID* taskData, INT context, UINT taskId, UINT taskCount);
		void TransformMeshes(UINT taskId, UINT taskCount, UINT idx);

		static void BinTransformedMeshes(VOID* taskData, INT context, UINT taskId, UINT taskCount);
		void BinTransformedMeshes(UINT taskId, UINT taskCount, UINT idx);

		static void SortBins(VOID* taskData, INT context, UINT taskId, UINT taskCount);

		static void RasterizeBinnedTrianglesToDepthBuffer(VOID* taskData, INT context, UINT taskId, UINT taskCount);
		void RasterizeBinnedTrianglesToDepthBuffer(UINT taskId, UINT idx);

		UINT mTileSequence[2][NUM_TILES];
};

#endif  //DEPTHBUFFERRASTERIZERAVXMT_H