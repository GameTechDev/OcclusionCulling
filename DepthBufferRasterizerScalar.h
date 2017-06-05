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
#ifndef DEPTHBUFFERRASTERIZERSCALAR_H
#define DEPTHBUFFERRASTERIZERSCALAR_H

#include "DepthBufferRasterizer.h"
#include "TransformedModelScalar.h"
#include "HelperScalar.h"

class DepthBufferRasterizerScalar : public DepthBufferRasterizer, public HelperScalar
{
	public:
		DepthBufferRasterizerScalar();
		virtual ~DepthBufferRasterizerScalar();

		void CreateTransformedModels(CPUTAssetSet **pAssetSet, UINT numAssetSets);

		// start inclusive, end exclusive
		void ClearDepthTile(int startX, int startY, int endX, int endY, UINT idx);

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
		
		inline void SetOccluderSizeThreshold(float occluderSizeThreshold) {mOccluderSizeThreshold = occluderSizeThreshold;}

		inline void SetCamera(CPUTCamera *pCamera, UINT idx) {mpCamera[idx] = pCamera;}

		inline void SetEnableFCulling(bool enableFCulling) {mEnableFCulling = enableFCulling;}

		inline const float *GetDepthSummaryBuffer(UINT idx){return NULL;}
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
		TransformedModelScalar* mpTransformedModels1;
		UINT mNumModels1;
		UINT *mpXformedPosOffset1;
		UINT *mpStartV1;
		UINT *mpStartT1;
		UINT mNumVertices1;
		UINT mNumTriangles1;
		UINT mNumRasterizedTris[2][NUM_TILES];
		float* mpXformedPos[2];
		CPUTCamera *mpCamera[2];
		float4x4 mpViewMatrix[2];
		float4x4 mpProjMatrix[2];
		UINT *mpRenderTargetPixels[2];
		UINT mNumRasterized[2];
		UINT	*mpBin[2];				 // triangle index
		USHORT  *mpBinModel[2];		 // model Index	
		USHORT  *mpBinMesh[2];			 // mesh index
		USHORT  *mpNumTrisInBin[2];     // number of triangles in the bin 
		UINT mTimeCounter;

		UINT *mpModelIndexA[2]; // 'active' models = visible and not too small
		UINT mNumModelsA[2];
		UINT mNumVerticesA[2];
		UINT mNumTrianglesA[2];

		float mOccluderSizeThreshold;

		bool   mEnableFCulling;
		double mRasterizeTime[AVG_COUNTER];
};

#endif  //DEPTHBUFFERRASTERIZERSCALAR_H