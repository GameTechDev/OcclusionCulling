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

#include "AABBoxRasterizerSSE.h"

struct AABBoxRasterizerSSE::WorldBBoxPacket
{
	__m128 mCenter[3];
	__m128 mHalf[3];

	inline void SetLane(UINT lane, const float3& center, const float3& half)
	{
		mCenter[0].m128_f32[lane] = center.x;
		mCenter[1].m128_f32[lane] = center.y;
		mCenter[2].m128_f32[lane] = center.z;
		mHalf[0].m128_f32[lane] = half.x;
		mHalf[1].m128_f32[lane] = half.y;
		mHalf[2].m128_f32[lane] = half.z;
	}
};

AABBoxRasterizerSSE::AABBoxRasterizerSSE()
	: mNumModels(0),
	  mpTransformedAABBox(NULL),
	  mpWorldBoxes(NULL),
	  mpModels(NULL),
	  mpNumTriangles(NULL),
	  mNumDepthTestTasks(0),
	  mOccludeeSizeThreshold(0.0f),
	  mTimeCounter(0),
	  mEnableFCulling(true)
{
	mpCamera[0] = mpCamera[1] = NULL;
	mpVisible[0] = mpVisible[1] = NULL;
	
	mViewMatrix[0] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mViewMatrix[1] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);

	mProjMatrix[0] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mProjMatrix[1] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);

	mpInsideFrustum[0] = mpInsideFrustum[1] = NULL;

	mpRenderTargetPixels[0] = NULL;
	mpRenderTargetPixels[1] = NULL;

	mpDepthSummary[0] = mpDepthSummary[1] = NULL;

	mNumCulled[0] = mNumCulled[1] = 0;
	for(UINT i = 0; i < AVG_COUNTER; i++)
	{
		mDepthTestTime[i] = 0.0;
	}
}

AABBoxRasterizerSSE::~AABBoxRasterizerSSE()
{
	for(UINT i = 0; i < mNumModels; i++)
	{
		mpModels[i]->Release();
	}
	_aligned_free(mViewMatrix[0]);
	_aligned_free(mViewMatrix[1]);
	_aligned_free(mProjMatrix[0]);
	_aligned_free(mProjMatrix[1]);
	_aligned_free(mpWorldBoxes);
	SAFE_DELETE_ARRAY(mpVisible[0]);
	SAFE_DELETE_ARRAY(mpVisible[1]);
	SAFE_DELETE_ARRAY(mpTransformedAABBox);
	SAFE_DELETE_ARRAY(mpInsideFrustum[0]);
	SAFE_DELETE_ARRAY(mpInsideFrustum[1]);
	SAFE_DELETE_ARRAY(mpNumTriangles);
	SAFE_DELETE_ARRAY(mpModels);
}

//--------------------------------------------------------------------
// * Go through the asset set and determine the model count in it
// * Create data structures aor all the models in the asset set
// * For each model create the axis aligned bounding box triangle 
//   vertex and index list
//--------------------------------------------------------------------
void AABBoxRasterizerSSE::CreateTransformedAABBoxes(CPUTAssetSet **pAssetSet, UINT numAssetSets)
{
	for(UINT assetId = 0;  assetId < numAssetSets; assetId++)
	{
		for(UINT nodeId = 0; nodeId < pAssetSet[assetId]->GetAssetCount(); nodeId++)
		{
			CPUTRenderNode* pRenderNode = NULL;
			CPUTResult result = pAssetSet[assetId]->GetAssetByIndex(nodeId, &pRenderNode);
			ASSERT((CPUT_SUCCESS == result), _L ("Failed getting asset by index")); 
			if(pRenderNode->IsModel())
			{
				mNumModels++;		
			}
			pRenderNode->Release();
		}
	}

	mpVisible[0] = new bool[mNumModels];
	mpVisible[1] = new bool[mNumModels];
	mpTransformedAABBox = new TransformedAABBoxSSE[mNumModels];
	mpModels = new CPUTModelDX11 *[mNumModels];

	UINT numPackets = (mNumModels + 3) / 4;
	mpWorldBoxes = (WorldBBoxPacket *)_aligned_malloc(numPackets * sizeof(WorldBBoxPacket), 16);
	memset(mpWorldBoxes, 0, numPackets * sizeof(WorldBBoxPacket));

	mpInsideFrustum[0] = new bool[numPackets * 4];
	mpInsideFrustum[1] = new bool[numPackets * 4];

	mpNumTriangles = new UINT[mNumModels];
	
	for(UINT assetId = 0, modelId = 0; assetId < numAssetSets; assetId++)
	{
		for(UINT nodeId = 0; nodeId < pAssetSet[assetId]->GetAssetCount(); nodeId++)
		{
			CPUTRenderNode* pRenderNode = NULL;
			CPUTResult result = pAssetSet[assetId]->GetAssetByIndex(nodeId, &pRenderNode);
			ASSERT((CPUT_SUCCESS == result), _L ("Failed getting asset by index")); 
			if(pRenderNode->IsModel())
			{
				CPUTModelDX11 *pModel = (CPUTModelDX11*)pRenderNode;
				pModel = (CPUTModelDX11*)pRenderNode;
	
				mpModels[modelId] = pModel;
				pModel->AddRef();

				mpTransformedAABBox[modelId].CreateAABBVertexIndexList(pModel);

				float3 bbCenter, bbHalf;
				pModel->GetBoundsWorldSpace(&bbCenter, &bbHalf);
				mpWorldBoxes[modelId / 4].SetLane(modelId & 3, bbCenter, bbHalf);

				mpNumTriangles[modelId] = 0;
				for(int meshId = 0; meshId < pModel->GetMeshCount(); meshId++)
				{
					mpNumTriangles[modelId] += pModel->GetMesh(meshId)->GetTriangleCount();
				}
				modelId++;
			}
			pRenderNode->Release();
		}
	}
}

void AABBoxRasterizerSSE::SetViewProjMatrix(float4x4 *viewMatrix, float4x4 *projMatrix, UINT idx)
{
	mViewMatrix[idx][0] = _mm_loadu_ps((float*)&viewMatrix->r0);
	mViewMatrix[idx][1] = _mm_loadu_ps((float*)&viewMatrix->r1);
	mViewMatrix[idx][2] = _mm_loadu_ps((float*)&viewMatrix->r2);
	mViewMatrix[idx][3] = _mm_loadu_ps((float*)&viewMatrix->r3);

	mProjMatrix[idx][0] = _mm_loadu_ps((float*)&projMatrix->r0);
	mProjMatrix[idx][1] = _mm_loadu_ps((float*)&projMatrix->r1);
	mProjMatrix[idx][2] = _mm_loadu_ps((float*)&projMatrix->r2);
	mProjMatrix[idx][3] = _mm_loadu_ps((float*)&projMatrix->r3);
}

//------------------------------------------------------------------------
// Go through the list of models in the asset set and render only those 
// models that are marked as visible by the software occlusion culling test
//------------------------------------------------------------------------
void AABBoxRasterizerSSE::RenderVisible(CPUTAssetSet **pAssetSet,
										CPUTRenderParametersDX &renderParams,
										UINT numAssetSets,
										UINT idx)
{
	int count = 0;

	for(UINT modelId = 0; modelId < mNumModels; modelId++)
	{
		if(mpVisible[idx][modelId])
		{
			mpModels[modelId]->Render(renderParams);
			count++;
		}
	}
	mNumCulled[idx] =  mNumModels - count;
}

//------------------------------------------------------------------------
// Go through the list of models in the asset set and render only those 
// models that are not marked as too small by the software occlusion culling test
//------------------------------------------------------------------------
void AABBoxRasterizerSSE::Render(CPUTAssetSet **pAssetSet,
								 CPUTRenderParametersDX &renderParams,
								 UINT numAssetSets,
								 UINT idx)
{
	int count = 0;
	int triCount = 0;

	CPUTModelDX11::ResetFCullCount();

	BoxTestSetupSSE setup;
	setup.Init(mViewMatrix[idx], mProjMatrix[idx], viewportMatrix, mpCamera[idx], mOccludeeSizeThreshold);

	__m128 cumulativeMatrix[4];

	for(UINT assetId = 0, modelId = 0; assetId < numAssetSets; assetId++)
	{
		for(UINT nodeId = 0; nodeId < pAssetSet[assetId]->GetAssetCount(); nodeId++)
		{
			CPUTRenderNode* pRenderNode = NULL;
			CPUTResult result = pAssetSet[assetId]->GetAssetByIndex(nodeId, &pRenderNode);
			ASSERT((CPUT_SUCCESS == result), _L ("Failed getting asset by index")); 
			if(pRenderNode->IsModel())
			{
				if(!mpTransformedAABBox[modelId].IsTooSmall(setup, cumulativeMatrix))
				{
					CPUTModelDX11* model = (CPUTModelDX11*)pRenderNode;
					ASSERT((model != NULL), _L("model is NULL"));

					model = (CPUTModelDX11*)pRenderNode;
					model->Render(renderParams);
					count++;
					triCount +=  mpNumTriangles[modelId];
				}
				modelId++;			
			}
			pRenderNode->Release();
		}
	}

	mNumFCullCount = CPUTModelDX11::GetFCullCount();
	mNumCulled[idx] = mNumModels - count;
	mNumTrisRendered = triCount;
}

//------------------------------------------------------------------------
// Calculate frustum culling state for a bunch of models
//------------------------------------------------------------------------
void AABBoxRasterizerSSE::CalcInsideFrustum(CPUTFrustum *pFrustum, UINT start, UINT end, UINT idx)
{
	// Packet start/end (rounding up)
	UINT packetStart = (start + 3) / 4;
	UINT packetEnd = (end + 3) / 4;

	// Prepare plane equations
	__m128 planeNormal[6][3];
	__m128 planeNormalSign[6][3];
	__m128 planeDist[6];
	__m128 signMask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
	for(UINT i = 0; i < 6; i++)
	{
		for (UINT j = 0; j < 3; j++)
		{
			planeNormal[i][j] = _mm_set1_ps(pFrustum->mpNormal[i].f[j]);
			planeNormalSign[i][j] = _mm_and_ps(planeNormal[i][j], signMask);
		}

		planeDist[i] = _mm_set1_ps(pFrustum->mPlanes[3*8 + i]);
	}

	// Loop over packets
	bool * __restrict visible = mpInsideFrustum[idx];
	for(UINT i = packetStart; i < packetEnd; i++)
	{
		// Start assuming all 4 boxes are inside
		__m128 inMask = _mm_castsi128_ps(_mm_set1_epi32(~0));

		// Loop over planes
		for (UINT j = 0; j < 6; j++)
		{
			// Sign for half[XYZ] so that dot product with plane normal would be maximal
			__m128 halfSignX = _mm_xor_ps(mpWorldBoxes[i].mHalf[0], planeNormalSign[j][0]);
			__m128 halfSignY = _mm_xor_ps(mpWorldBoxes[i].mHalf[1], planeNormalSign[j][1]);
			__m128 halfSignZ = _mm_xor_ps(mpWorldBoxes[i].mHalf[2], planeNormalSign[j][2]);

			// Bounding box corner to test (min corner)
			__m128 cornerX = _mm_sub_ps(mpWorldBoxes[i].mCenter[0], halfSignX);
			__m128 cornerY = _mm_sub_ps(mpWorldBoxes[i].mCenter[1], halfSignY);
			__m128 cornerZ = _mm_sub_ps(mpWorldBoxes[i].mCenter[2], halfSignZ);

			// Compute dot product
			__m128 dot = planeDist[j];
			dot = _mm_add_ps(dot, _mm_mul_ps(cornerX, planeNormal[j][0]));
			dot = _mm_add_ps(dot, _mm_mul_ps(cornerY, planeNormal[j][1]));
			dot = _mm_add_ps(dot, _mm_mul_ps(cornerZ, planeNormal[j][2]));

			// The plane box is inside as long as the dot product is negative -> sign bit set
			// So AND result together with current mask
			inMask = _mm_and_ps(inMask, dot);
		}

		// Write the results for this packet
		int packetMask = _mm_movemask_ps(inMask);
		visible[i*4 + 0] = (packetMask >> 0) & 1;
		visible[i*4 + 1] = (packetMask >> 1) & 1;
		visible[i*4 + 2] = (packetMask >> 2) & 1;
		visible[i*4 + 3] = (packetMask >> 3) & 1;
	}
}