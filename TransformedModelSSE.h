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

#ifndef TRANSFORMEDMODELSSE_H
#define TRANSFORMEDMODELSSE_H

#include "CPUT_DX11.h"
#include "TransformedMeshSSE.h"
#include "HelperSSE.h"

struct BoxTestSetupSSE;
class MaskedOcclusionCulling;
class CullingThreadpool;

class TransformedModelSSE : public HelperSSE
{
	public:
		TransformedModelSSE();
		~TransformedModelSSE();
		void CreateTransformedMeshes(CPUTModelDX11 *pModel);
		void InsideViewFrustum(const BoxTestSetupSSE &setup,
							   UINT idx);

		void TooSmall(const BoxTestSetupSSE &setup,
					  UINT idx);

		void TransformMeshes(UINT start, 
							 UINT end,
							 CPUTCamera *pCamera,
							 UINT idx);

		void BinTransformedTrianglesST(UINT taskId,
									   UINT modelId,
									   UINT start,
									   UINT end,
									   BinTriangle* pBin,
									   USHORT* pNumTrisInBin,
									   UINT idx);

		void BinTransformedTrianglesMT(UINT taskId,
									   UINT modelId,
									   UINT start,
									   UINT end,
									   BinTriangle* pBin,
									   USHORT* pNumTrisInBin,
									   UINT idx);
		void TransformAndRasterizeTrianglesST(MaskedOcclusionCulling *moc, UINT idx);
        void TransformAndRasterizeTrianglesMT(CullingThreadpool *mocThreadpool, UINT idx );

		inline UINT GetNumVertices(){return mNumVertices;}

		inline UINT GetNumTriangles(){return mNumTriangles;}

		inline void SetXformedPos(__m128 *pXformedPos0, __m128 *pXformedPos1, UINT modelStart)
		{
			mpXformedPos[0] = pXformedPos0;
			mpXformedPos[1] = pXformedPos1;

			mpMeshes[0].SetXformedPos(mpXformedPos[0], mpXformedPos[1]);
					
			UINT numVertices = 0;
			numVertices += mpMeshes[0].GetNumVertices();
			
			for(UINT i = 1; i < mNumMeshes; i++)
			{
				mpMeshes[i].SetXformedPos(mpXformedPos[0] + numVertices, mpXformedPos[1] + numVertices);
				numVertices += mpMeshes[i].GetNumVertices(); 
			}
		}
		
		inline void SetInsideFrustum(bool inFrustum, UINT idx){mInsideViewFrustum[idx] = inFrustum;}

		inline bool IsRasterized2DB(UINT idx)
		{
			return (mInsideViewFrustum[idx] && !mTooSmall[idx]);
		}

		inline float GetDepth() { return mBBCenterW; }

	private:
		CPUTModelDX11 *mpCPUTModel;
		UINT mNumMeshes;
		__m128 *mWorldMatrix;
		__m128 *mCumulativeMatrix[2];
		UINT mNumVertices;
		UINT mNumTriangles;
				
		float3 mBBCenterWS;
		float3 mBBHalfWS;
		bool mInsideViewFrustum[2];
		bool mTooSmall[2];

		float3 mBBCenterOS;
		float mRadiusSq;
		TransformedMeshSSE *mpMeshes;
		__m128 *mpXformedPos[2];	

		float mBBCenterW;
};

#endif