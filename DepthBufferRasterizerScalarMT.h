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
#ifndef DEPTHBUFFERRASTERIZERSCALARMT_H
#define DEPTHBUFFERRASTERIZERSCALARMT_H

#include "DepthBufferRasterizerScalar.h"

class DepthBufferRasterizerScalarMT : public DepthBufferRasterizerScalar
{
	public:
		DepthBufferRasterizerScalarMT();
		~DepthBufferRasterizerScalarMT();

		struct PerTaskData
		{
			UINT idx;
			DepthBufferRasterizerScalarMT *pDBR; 
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
		void RasterizeBinnedTrianglesToDepthBuffer(UINT rawTaskId, UINT idx);

		UINT mTileSequence[2][NUM_TILES];
};

#endif  //DEPTHBUFFERRASTERIZERSCALARMT_H