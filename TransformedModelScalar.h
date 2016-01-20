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