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
//
//--------------------------------------------------------------------------------------

#ifndef AABBOXRASTERIZERSSE_H
#define AABBOXRASTERIZERSSE_H

#include "AABBoxRasterizer.h"
#include "TransformedAABBoxSSE.h"

class AABBoxRasterizerSSE : public AABBoxRasterizer
{
	public:
		AABBoxRasterizerSSE();
		virtual ~AABBoxRasterizerSSE();
		void CreateTransformedAABBoxes(CPUTAssetSet **pAssetSet, UINT numAssetSets);
		
		void RenderVisible(CPUTAssetSet **pAssetSet,
						   CPUTRenderParametersDX &renderParams,
						   UINT numAssetsSets, 
						   UINT idx);

		void Render(CPUTAssetSet **pAssetSet,
					CPUTRenderParametersDX &renderParams,
					UINT numAssetSets,
					UINT idx);

		inline void ResetInsideFrustum()
		{
			for(UINT i = 0; i < mNumModels; i++)
			{
				mpInsideFrustum[0][i] = true;
				mpInsideFrustum[1][i] = true;
			}
		}

		void SetViewProjMatrix(float4x4 *viewMatrix, float4x4 *projMatrix, UINT idx);
		inline void SetCPURenderTargetPixels(UINT *pRenderTargetPixels, UINT idx)
		{
			mpRenderTargetPixels[idx] = pRenderTargetPixels;
		}
		inline void SetDepthSummaryBuffer(const float *pDepthSummary, UINT idx){mpDepthSummary[idx] = pDepthSummary;}
		inline void SetDepthTestTasks(UINT numTasks) {mNumDepthTestTasks = numTasks;}
		inline void SetOccludeeSizeThreshold(float occludeeSizeThreshold){mOccludeeSizeThreshold = occludeeSizeThreshold;}
		inline void SetCamera(CPUTCamera *pCamera, UINT idx) {mpCamera[idx] = pCamera;}
		inline void SetEnableFCulling(bool enableFCulling) {mEnableFCulling = enableFCulling;}

		inline UINT GetNumOccludees() {return mNumModels;}
		inline UINT GetNumCulled(UINT idx) {return mNumCulled[idx];}
		inline double GetDepthTestTime()
		{
			double averageTime = 0.0;
			for(UINT i = 0; i < AVG_COUNTER; i++)
			{
				averageTime += mDepthTestTime[i];
			}
			return averageTime / AVG_COUNTER;
		}

		inline UINT GetNumTriangles()
		{
			UINT numTris = 0;
			for(UINT i = 0; i < mNumModels; i++)
			{
				numTris += mpNumTriangles[i];
			}
			return numTris;
		}

		inline UINT GetNumCulledTriangles(UINT idx)
		{
			UINT numCulledTris = 0;
			for(UINT i = 0; i < mNumModels; i++)
			{
				numCulledTris += mpVisible[idx][i] ? 0 : mpNumTriangles[i];
			}
			return numCulledTris;
		}

		inline UINT GetNumTrisRendered()
		{
			return mNumTrisRendered;
		}

		inline UINT GetNumFCullCount()
		{
			return mNumFCullCount;
		}

		void CalcInsideFrustum(CPUTFrustum *pFrustum, UINT start, UINT end, UINT idx);

	protected:
		struct WorldBBoxPacket;

		UINT mNumModels;
		TransformedAABBoxSSE *mpTransformedAABBox;
		WorldBBoxPacket *mpWorldBoxes;
		CPUTModelDX11 **mpModels;
		bool *mpInsideFrustum[2];
		UINT *mpNumTriangles;
		__m128 *mViewMatrix[2];
		__m128 *mProjMatrix[2];
		UINT *mpRenderTargetPixels[2];
		CPUTCamera *mpCamera[2];
		bool *mpVisible[2];
		UINT mNumCulled[2];
		UINT mNumTrisRendered;
		UINT mNumFCullCount;
		UINT mNumDepthTestTasks;
		float mOccludeeSizeThreshold;
		UINT mTimeCounter;
		
		const float *mpDepthSummary[2];
		bool mEnableFCulling;
		double mDepthTestTime[AVG_COUNTER];
};



#endif //AABBOXRASTERIZERSSE_H