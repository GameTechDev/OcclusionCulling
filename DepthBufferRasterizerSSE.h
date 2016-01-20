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
#ifndef DEPTHBUFFERRASTERIZERSSE_H
#define DEPTHBUFFERRASTERIZERSSE_H

#include "DepthBufferRasterizer.h"
#include "TransformedModelSSE.h"
#include "HelperSSE.h"

union XY
{
	struct
	{
		short x, y;
	};
	unsigned int xy;
};

struct BinTriangle
{
	XY vert[3];
	float Z[3];  // Plane equation, not just values at the three verts
};


class DepthBufferRasterizerSSE : public DepthBufferRasterizer, public HelperSSE
{
	public:
		DepthBufferRasterizerSSE();
		virtual ~DepthBufferRasterizerSSE();
		
		void CreateTransformedModels(CPUTAssetSet **pAssetSet, UINT numAssetSets);

		// start inclusive, end exclusive
		void ClearDepthTile(int startX, int startY, int endX, int endY, UINT idx);
		void CreateCoarseDepth(int startX, int startY, int endX, int endY, UINT idx);
				
		// Reset all models to be visible when frustum culling is disabled 
		inline void ResetInsideFrustum()
		{
			for(UINT i = 0; i < mNumModels1; i++)
			{
				mpTransformedModels1[i].SetInsideFrustum(true, 0);
				mpTransformedModels1[i].SetInsideFrustum(true, 1);
			}
		}

		// Set the view and projection matrices
		inline void SetViewProj(float4x4 *viewMatrix, float4x4 *projMatrix, UINT idx);
		
		inline void SetCPURenderTargetPixels(UINT *pRenderTargetPixels, UINT idx)
		{
			mpRenderTargetPixels[idx] = pRenderTargetPixels;
		}
		inline const float *GetDepthSummaryBuffer(UINT idx) {return mpSummaryBuffer[idx];}
		inline void SetCamera(CPUTCamera *pCamera, UINT idx) {mpCamera[idx] = pCamera;}

		inline void SetOccluderSizeThreshold(float occluderSizeThreshold) {mOccluderSizeThreshold = occluderSizeThreshold;}

		inline void SetEnableFCulling(bool enableFCulling) {mEnableFCulling = enableFCulling;}

		inline UINT GetNumOccluders() {return mNumModels1;}
		inline UINT GetNumOccludersR2DB(UINT idx)
		{
			mNumRasterized[idx] = 0;
			for(UINT i = 0; i < mNumModels1; i++)
			{
				mNumRasterized[idx] += mpTransformedModels1[i].IsRasterized2DB(idx) ? 1 : 0;
			}
			return mNumRasterized[idx];
		}
		inline double GetRasterizeTime()
		{
			double averageTime = 0.0;
			for(UINT i = 0; i < AVG_COUNTER; i++)
			{
				averageTime += mRasterizeTime[i];
			}
			return averageTime / AVG_COUNTER;
		}
		inline UINT GetNumTriangles(){return mNumTriangles1;}
		inline UINT GetNumRasterizedTriangles(UINT idx) 
		{
			UINT numRasterizedTris = 0;
			for(UINT i = 0; i < NUM_TILES; i++)
			{
				numRasterizedTris += mNumRasterizedTris[idx][i];
			}
			return numRasterizedTris;
		}

		inline void ResetActive(UINT idx)
		{
			mNumModelsA[idx] = mNumVerticesA[idx] = mNumTrianglesA[idx] = 0;
		}

		inline void Activate(UINT modelId, UINT idx)
		{
			UINT activeId = mNumModelsA[idx]++;
			assert(activeId < mNumModels1);

			mpModelIndexA[idx][activeId] = modelId;
			mNumVerticesA[idx] += mpStartV1[modelId + 1] - mpStartV1[modelId];
			mNumTrianglesA[idx] += mpStartT1[modelId + 1] - mpStartT1[modelId];
		}
		
	protected:
		TransformedModelSSE *mpTransformedModels1;
		UINT mNumModels1;
		UINT *mpXformedPosOffset1;
		UINT *mpStartV1;
		UINT *mpStartT1;
		UINT mNumVertices1;
		UINT mNumTriangles1;
		UINT mNumRasterizedTris[2][NUM_TILES];
		__m128 *mpXformedPos[2];
		CPUTCamera *mpCamera[2];
		__m128 *mpViewMatrix[2];
		__m128 *mpProjMatrix[2];
		UINT *mpRenderTargetPixels[2];
		UINT mNumRasterized[2];
		BinTriangle *mpBin[2];
		USHORT *mpNumTrisInBin[2];      // number of triangles in the bin
		UINT mTimeCounter;

		UINT *mpModelIndexA[2]; // 'active' models = visible and not too small
		UINT mNumModelsA[2];
		UINT mNumVerticesA[2];
		UINT mNumTrianglesA[2];

		float mOccluderSizeThreshold;
		float *mpSummaryBuffer[2];

		bool   mEnableFCulling;
		double mRasterizeTime[AVG_COUNTER];
};

#endif  //DEPTHBUFFERRASTERIZERSSE_H