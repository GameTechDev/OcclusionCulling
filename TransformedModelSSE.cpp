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
#include "TransformedModelSSE.h"

TransformedModelSSE::TransformedModelSSE()
	: mpCPUTModel(NULL),
	  mNumMeshes(0),
	  mWorldMatrix(NULL),
	  mNumVertices(0),
	  mNumTriangles(0),
	  mpMeshes(NULL)
{
	mInsideViewFrustum[0] = mInsideViewFrustum[1] = false;
	mTooSmall[0] = mTooSmall[1] = false;

	mpXformedPos[0] = mpXformedPos[1] = NULL;
	mWorldMatrix = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mCumulativeMatrix[0] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mCumulativeMatrix[1] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
}

TransformedModelSSE::~TransformedModelSSE()
{
	SAFE_DELETE_ARRAY(mpMeshes);
	_aligned_free(mWorldMatrix);
	_aligned_free(mCumulativeMatrix[0]);
	_aligned_free(mCumulativeMatrix[1]);
}

//--------------------------------------------------------------------
// Create place holder for the transformed meshes for each model
//---------------------------------------------------------------------
void TransformedModelSSE::CreateTransformedMeshes(CPUTModelDX11 *pModel)
{
	mpCPUTModel = pModel;
	mNumMeshes = pModel->GetMeshCount();

	float *world = (float*)pModel->GetWorldMatrix();
	mWorldMatrix[0] = _mm_loadu_ps(world + 0);
	mWorldMatrix[1] = _mm_loadu_ps(world + 4);
	mWorldMatrix[2] = _mm_loadu_ps(world + 8);
	mWorldMatrix[3] = _mm_loadu_ps(world + 12);
		
	float3 center, half;
	pModel->GetBoundsObjectSpace(&center, &half);

	mBBCenterOS = center;
	mRadiusSq = half.lengthSq();
	mpMeshes = new TransformedMeshSSE[mNumMeshes];

	for(UINT i = 0; i < mNumMeshes; i++)
	{
		CPUTMeshDX11* pMesh = (CPUTMeshDX11*)pModel->GetMesh(i);
		ASSERT((pMesh != NULL), _L("pMesh is NULL"));

		mpMeshes[i].Initialize(pMesh);
		mNumVertices += mpMeshes[i].GetNumVertices();
		mNumTriangles += mpMeshes[i].GetNumTriangles();
	}
}


void TransformedModelSSE::TooSmall(const BoxTestSetupSSE &setup, UINT idx)
{
	if(mInsideViewFrustum[idx])
	{
		MatrixMultiply(mWorldMatrix, setup.mViewProjViewport, mCumulativeMatrix[idx]);

		float w = mBBCenterOS.x * mCumulativeMatrix[idx][0].m128_f32[3] +
				  mBBCenterOS.y * mCumulativeMatrix[idx][1].m128_f32[3] +
				  mBBCenterOS.z * mCumulativeMatrix[idx][2].m128_f32[3] +
				  mCumulativeMatrix[idx][3].m128_f32[3]; 

		if(w > 1.0f)
		{
			mTooSmall[idx] = mRadiusSq < w * setup.radiusThreshold;
		}
		else
		{
			// BB center is behind the near clip plane, making screen-space radius meaningless.
            // Assume visible.  This should be a safe assumption, as the frustum test says the bbox is visible.
            mTooSmall[idx] = false;
        }
	}
}


//------------------------------------------------------------------
// Determine is the occluder model is inside view frustum
//------------------------------------------------------------------
void TransformedModelSSE::InsideViewFrustum(const BoxTestSetupSSE &setup, UINT idx)
{
	mpCPUTModel->GetBoundsWorldSpace(&mBBCenterWS, &mBBHalfWS);
	mInsideViewFrustum[idx] = setup.mpCamera->mFrustum.IsVisible(mBBCenterWS, mBBHalfWS);

	if(mInsideViewFrustum[idx])
	{
		MatrixMultiply(mWorldMatrix, setup.mViewProjViewport, mCumulativeMatrix[idx]);

		float w = mBBCenterOS.x * mCumulativeMatrix[idx][0].m128_f32[3] +
				  mBBCenterOS.y * mCumulativeMatrix[idx][1].m128_f32[3] +
				  mBBCenterOS.z * mCumulativeMatrix[idx][2].m128_f32[3] +
				  mCumulativeMatrix[idx][3].m128_f32[3]; 

		if(w > 1.0f)
		{
			mTooSmall[idx] = mRadiusSq < w * setup.radiusThreshold;
		}
		else
		{
			// BB center is behind the near clip plane, making screen-space radius meaningless.
            // Assume visible.  This should be a safe assumption, as the frustum test says the bbox is visible.
            mTooSmall[idx] = false;
        }
	}
}

//---------------------------------------------------------------------------------------------------
// Determine if the occluder size is sufficiently large enough to occlude other object sin the scene
// If so transform the occluder to screen space so that it can be rasterized to the cPU depth buffer
//---------------------------------------------------------------------------------------------------
void TransformedModelSSE::TransformMeshes(UINT start, 
										  UINT end,
										  CPUTCamera* pCamera,
										  UINT idx)
{
	if(mInsideViewFrustum[idx] && !mTooSmall[idx])
	{
		UINT totalNumVertices = 0;
		for(UINT meshId = 0; meshId < mNumMeshes; meshId++)
		{
			totalNumVertices +=  mpMeshes[meshId].GetNumVertices();
			if(totalNumVertices < start)
		    {
				continue;
			}
			mpMeshes[meshId].TransformVertices(mCumulativeMatrix[idx], start, end, idx);
		}
	}
}

//------------------------------------------------------------------------------------
// If the occluder is sufficiently large enough to occlude other objects in the scene 
// bin the triangles that make up the occluder into tiles to speed up rateraization
// Single threaded version
//------------------------------------------------------------------------------------
void TransformedModelSSE::BinTransformedTrianglesST(UINT taskId,
												    UINT modelId,
											        UINT start,
											        UINT end,
												    BinTriangle* pBin,
												    USHORT* pNumTrisInBin,
													UINT idx)
{
	if(mInsideViewFrustum[idx] && !mTooSmall[idx])
	{
		UINT totalNumTris = 0;
		for(UINT meshId = 0; meshId < mNumMeshes; meshId++)
		{
			totalNumTris += mpMeshes[meshId].GetNumTriangles();
			if(totalNumTris < start)
			{
				continue;
			}

			mpMeshes[meshId].BinTransformedTrianglesST(taskId, modelId, meshId, start, end, pBin, pNumTrisInBin, idx);
		}
	}
}

//------------------------------------------------------------------------------------
// If the occluder is sufficiently large enough to occlude other objects in the scene 
// bin the triangles that make up the occluder into tiles to speed up rateraization
// Multi threaded version
//------------------------------------------------------------------------------------
void TransformedModelSSE::BinTransformedTrianglesMT(UINT taskId,
												    UINT modelId,
											        UINT start,
											        UINT end,
												    BinTriangle* pBin,
												    USHORT* pNumTrisInBin,
													UINT idx)
{
	if(mInsideViewFrustum[idx] && !mTooSmall[idx])
	{
		UINT totalNumTris = 0;
		for(UINT meshId = 0; meshId < mNumMeshes; meshId++)
		{
			totalNumTris += mpMeshes[meshId].GetNumTriangles();
			if(totalNumTris < start)
			{
				continue;
			}

			mpMeshes[meshId].BinTransformedTrianglesMT(taskId, modelId, meshId, start, end, pBin, pNumTrisInBin, idx);
		}
	}
}