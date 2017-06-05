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

#ifndef TRANSFORMEDMODELSCALAR_H
#define TRANSFORMEDMODELSCALAR_H

#include "CPUT_DX11.h"
#include "TransformedMeshScalar.h"
#include "HelperScalar.h"

class TransformedModelScalar : public HelperScalar
{
	public:
		TransformedModelScalar();
		~TransformedModelScalar();
		void CreateTransformedMeshes(CPUTModelDX11 *pModel);
		void InsideViewFrustum(const BoxTestSetupScalar &setup,
							   UINT idx);

		void TooSmall(const BoxTestSetupScalar &setup,
					  UINT idx);

		void TransformMeshes(UINT start, 
							 UINT end,
							 CPUTCamera* pCamera,
							 UINT idx);

		void BinTransformedTrianglesST(UINT taskId,
			   					       UINT modelId,
									   UINT start,
									   UINT end,
									   UINT* pBin,
									   USHORT* pBinModel,
									   USHORT* pBinMesh,
									   USHORT* pNumTrisInBin,
									   UINT idx);

		void BinTransformedTrianglesMT(UINT taskId,
			   					       UINT modelId,
									   UINT start,
									   UINT end,
									   UINT* pBin,
									   USHORT* pBinModel,
									   USHORT* pBinMesh,
									   USHORT* pNumTrisInBin,
									   UINT idx);

		void Gather(float* xformedPos,
					UINT meshId, 
					UINT triId, 
					UINT idx);

		inline UINT GetNumVertices(){return mNumVertices;}

		inline UINT GetNumTriangles(){return mNumTriangles;}

		inline void SetXformedPos(float4 *pXformedPos0, float4 *pXformedPos1, UINT modelStart)
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
	
	private:
		CPUTModelDX11 *mpCPUTModel;
		UINT mNumMeshes;
		float4x4 mWorldMatrix;
		float4x4 mCumulativeMatrix[2];
		UINT mNumVertices;
		UINT mNumTriangles;

		float3 mBBCenterWS;
		float3 mBBHalfWS;
		bool mInsideViewFrustum[2];
		bool mTooSmall[2];
		float mOccluderSizeThreshold;

		float3 mBBCenterOS;
		float mRadiusSq;
		TransformedMeshScalar *mpMeshes;
		float4 *mpXformedPos[2];
};

#endif