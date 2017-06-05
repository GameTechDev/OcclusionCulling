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

#ifndef AABBOXRASTERIZERSCALAR_H
#define AABBOXRASTERIZERSCALAR_H

#include "AABBoxRasterizer.h"
#include "TransformedAABBoxScalar.h"

class AABBoxRasterizerScalar : public AABBoxRasterizer
{
	public:
		AABBoxRasterizerScalar();
		virtual ~AABBoxRasterizerScalar();
		void CreateTransformedAABBoxes(CPUTAssetSet **pAssetSet, UINT numAssetSets);
		
		void RenderVisible(CPUTAssetSet **pAssetSet,
						   CPUTRenderParametersDX &renderParams,
						   UINT numAssetSets,
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
		inline void SetDepthSummaryBuffer(const float *pSummaryBuffer, UINT idx){}
		inline void SetDepthTestTasks(UINT numTasks){mNumDepthTestTasks = numTasks;}
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

	protected:
		UINT mNumModels;
		TransformedAABBoxScalar *mpTransformedAABBox;
		CPUTModelDX11 **mpModels;
		bool *mpInsideFrustum[2];
		UINT *mpNumTriangles;
		float4x4 mViewMatrix[2];
		float4x4 mProjMatrix[2];
		UINT *mpRenderTargetPixels[2];
		CPUTCamera *mpCamera[2];
		bool *mpVisible[2];
		UINT mNumCulled[2];
		UINT mNumTrisRendered;
		UINT mNumFCullCount;
		UINT mNumDepthTestTasks;
		float mOccludeeSizeThreshold;
		UINT mTimeCounter;

		bool   mEnableFCulling;
		double mDepthTestTime[AVG_COUNTER];
};



#endif //AABBOXRASTERIZERSCALAR_H