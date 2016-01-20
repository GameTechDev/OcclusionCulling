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
//-------------------------------------------------------------------------------------
#include "DepthBufferRasterizerSSE.h"

DepthBufferRasterizerSSE::DepthBufferRasterizerSSE()
	: DepthBufferRasterizer(),
	  mpTransformedModels1(NULL),
	  mNumModels1(0),
	  mpXformedPosOffset1(NULL),
	  mpStartV1(NULL),
	  mpStartT1(NULL),
	  mNumVertices1(0),
	  mNumTriangles1(0),
	  mOccluderSizeThreshold(0.0f),
	  mTimeCounter(0),
	  mEnableFCulling(true)
{
	mpXformedPos[0] = mpXformedPos[1] = NULL;
	mpCamera[0] = mpCamera[1] = NULL;
	mpViewMatrix[0] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mpViewMatrix[1] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mpProjMatrix[0] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mpProjMatrix[1] = (__m128*)_aligned_malloc(sizeof(float) * 4 * 4, 16);
	mpRenderTargetPixels[0] = NULL;
	mpRenderTargetPixels[1] = NULL;
	mpSummaryBuffer[0] = (float*)_aligned_malloc(sizeof(float) * (SCREENW/8) * (SCREENH/8), 64);
	mpSummaryBuffer[1] = (float*)_aligned_malloc(sizeof(float) * (SCREENW/8) * (SCREENH/8), 64);

	mNumRasterized[0] = mNumRasterized[1] = NULL;

	mpBin[0] = mpBin[1] = NULL;
	
	mpNumTrisInBin[0] = mpNumTrisInBin[1] = NULL;
	
	mpModelIndexA[0] = mpModelIndexA[1] = NULL;
	
	for(UINT i = 0; i < AVG_COUNTER; i++)
	{
		mRasterizeTime[i] = 0.0;
	}
}

DepthBufferRasterizerSSE::~DepthBufferRasterizerSSE()
{
	SAFE_DELETE_ARRAY(mpTransformedModels1);
	SAFE_DELETE_ARRAY(mpXformedPosOffset1);
	SAFE_DELETE_ARRAY(mpStartV1);
	SAFE_DELETE_ARRAY(mpStartT1);
	SAFE_DELETE_ARRAY(mpModelIndexA[0]);
	SAFE_DELETE_ARRAY(mpModelIndexA[1]);
	_aligned_free(mpXformedPos[0]);
	_aligned_free(mpXformedPos[1]);
	_aligned_free(mpViewMatrix[0]);
	_aligned_free(mpViewMatrix[1]);
	_aligned_free(mpProjMatrix[0]);
	_aligned_free(mpProjMatrix[1]);
	_aligned_free(mpSummaryBuffer[0]);
	_aligned_free(mpSummaryBuffer[1]);
}

//--------------------------------------------------------------------
// * Go through the asset set and determine the model count in it
// * Create data structures for all the models in the asset set
// * For each model create the place holders for the transformed vertices
//--------------------------------------------------------------------
void DepthBufferRasterizerSSE::CreateTransformedModels(CPUTAssetSet **mpAssetSet, UINT numAssetSets)
{
	for(UINT assetId = 0; assetId < numAssetSets; assetId++)
	{
		for(UINT nodeId = 0; nodeId < mpAssetSet[assetId]->GetAssetCount(); nodeId++)
		{
			CPUTRenderNode* pRenderNode = NULL;
			CPUTResult result = mpAssetSet[assetId]->GetAssetByIndex(nodeId, &pRenderNode);
			ASSERT((CPUT_SUCCESS == result), _L ("Failed getting asset by index")); 
			if(pRenderNode->IsModel())
			{
				mNumModels1++;		
			}
			pRenderNode->Release();
		}
	}

	mpTransformedModels1 = new TransformedModelSSE[mNumModels1];
	mpXformedPosOffset1 = new UINT[mNumModels1];
	mpStartV1 = new UINT[mNumModels1 + 1];
	mpStartT1 = new UINT[mNumModels1 + 1];

	//mpStartV1[0] = mpStartT1[0] = 0;
	mpModelIndexA[0] = new UINT[mNumModels1];
	mpModelIndexA[1] = new UINT[mNumModels1];
	UINT modelId = 0;

	for(UINT assetId = 0; assetId < numAssetSets; assetId++)
	{
		for(UINT nodeId = 0; nodeId < mpAssetSet[assetId]->GetAssetCount(); nodeId++)
		{
			CPUTRenderNode* pRenderNode = NULL;
			CPUTResult result = mpAssetSet[assetId]->GetAssetByIndex(nodeId, &pRenderNode);
			ASSERT((CPUT_SUCCESS == result), _L ("Failed getting asset by index")); 
			if(pRenderNode->IsModel())
			{
				CPUTModelDX11* model = (CPUTModelDX11*)pRenderNode;
				ASSERT((model != NULL), _L("model is NULL"));

				model = (CPUTModelDX11*)pRenderNode;
				mpTransformedModels1[modelId].CreateTransformedMeshes(model);
			
				mpXformedPosOffset1[modelId] = mpTransformedModels1[modelId].GetNumVertices();
								
				mpStartV1[modelId] = mNumVertices1;				
				mNumVertices1 += mpTransformedModels1[modelId].GetNumVertices();

				mpStartT1[modelId] = mNumTriangles1;
				mNumTriangles1 += mpTransformedModels1[modelId].GetNumTriangles();
				modelId++;
			}
			pRenderNode->Release();
		}
	}

	mpStartV1[modelId] = mNumVertices1;
	mpStartT1[modelId] = mNumTriangles1;
		
	//for x, y, z, w
	mpXformedPos[0] = (__m128*)_aligned_malloc(sizeof(float)* 4 * mNumVertices1, 16);
	mpXformedPos[1] = (__m128*)_aligned_malloc(sizeof(float)* 4 * mNumVertices1, 16);

	for(UINT i = 0; i < mNumModels1; i++)
	{
		mpTransformedModels1[i].SetXformedPos(&mpXformedPos[0][mpStartV1[i]],
											  &mpXformedPos[1][mpStartV1[i]],
											  mpStartV1[i]);
	}
}

//--------------------------------------------------------------------
// Clear depth buffer for a tile
//--------------------------------------------------------------------
void DepthBufferRasterizerSSE::ClearDepthTile(int startX, int startY, int endX, int endY, UINT idx)
{
	assert(startX % 2 == 0 && startY % 2 == 0);
	assert(endX % 2 == 0 && endY % 2 == 0);

	float* pDepthBuffer = (float*)mpRenderTargetPixels[idx];
	int width = endX - startX;

	// Note we need to account for tiling pattern here
	for(int r = startY; r < endY; r += 2)
	{
		int rowIdx = r * SCREENW + 2 * startX;
		memset(&pDepthBuffer[rowIdx], 0, sizeof(float) * 2 * width);
	}
}

//--------------------------------------------------------------------
// Summarize the depth buffer for a tile
//--------------------------------------------------------------------
void DepthBufferRasterizerSSE::CreateCoarseDepth(int startX, int startY, int endX, int endY, UINT idx)
{
	assert(startX % BLOCK_SIZE == 0 && startY % BLOCK_SIZE == 0);
	assert(endX % BLOCK_SIZE == 0 && endY % BLOCK_SIZE == 0);

	//creat 8x8 blocks and find the minimum depth for each 8x8 block   
	const float* pDepthBuffer = (const float*)mpRenderTargetPixels[idx];
	int x0s = startX / BLOCK_SIZE;
	int y0s = startY / BLOCK_SIZE;
	int x1s = endX / BLOCK_SIZE;
	int y1s = endY / BLOCK_SIZE;

	for(int yt = y0s; yt < y1s; yt++)
	{
		const float *srcRow = pDepthBuffer + (yt * BLOCK_SIZE) * SCREENW;
		float *dstRow = mpSummaryBuffer[idx] + yt * (SCREENW/BLOCK_SIZE);

		for(int xt = x0s; xt <= x1s; xt++)
		{
			const float *src = srcRow + (xt * BLOCK_SIZE) * 2;
			
			static const int ofsTab[8] = 
			{
				0*SCREENW, 0*SCREENW + BLOCK_SIZE,
				2*SCREENW, 2*SCREENW + BLOCK_SIZE,
				4*SCREENW, 4*SCREENW + BLOCK_SIZE,
				6*SCREENW, 6*SCREENW + BLOCK_SIZE
			};
			__m128 min0 = _mm_set1_ps(1.0f);
			__m128 min1 = _mm_set1_ps(1.0f);
	
			for(int i = 0; i < 8; i++)
			{
				const float *srcQuad = src + ofsTab[i];
				// find minimum within the first 4 values of the 8x8 block
				min0 = _mm_min_ps(min0, _mm_load_ps(srcQuad + 0));
				// find minimum within the second 4 values of the 8x8 block
				min1 = _mm_min_ps(min1, _mm_load_ps(srcQuad + SSE));
			}

			// merge the two minimums
			min0 = _mm_min_ps(min0, min1);
			min0 = _mm_min_ps(min0, _mm_shuffle_ps(min0, min0, 0x4e)); // .zwxy
			min0 = _mm_min_ps(min0, _mm_shuffle_ps(min0, min0, 0xb1)); // .yxwz
			_mm_store_ss(&dstRow[xt], min0);
		}
	}
}

void DepthBufferRasterizerSSE::SetViewProj(float4x4 *viewMatrix, float4x4 *projMatrix, UINT idx)
{
	mpViewMatrix[idx][0] = _mm_loadu_ps((float*)&viewMatrix->r0);
	mpViewMatrix[idx][1] = _mm_loadu_ps((float*)&viewMatrix->r1);
	mpViewMatrix[idx][2] = _mm_loadu_ps((float*)&viewMatrix->r2);
	mpViewMatrix[idx][3] = _mm_loadu_ps((float*)&viewMatrix->r3);

	mpProjMatrix[idx][0] = _mm_loadu_ps((float*)&projMatrix->r0);
	mpProjMatrix[idx][1] = _mm_loadu_ps((float*)&projMatrix->r1);
	mpProjMatrix[idx][2] = _mm_loadu_ps((float*)&projMatrix->r2);
	mpProjMatrix[idx][3] = _mm_loadu_ps((float*)&projMatrix->r3);
}