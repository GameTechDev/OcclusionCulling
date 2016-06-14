//--------------------------------------------------------------------------------------
// Copyright 2015 Intel Corporation
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
#include "DepthBufferMaskedRasterizerAVXST.h"
#include "MaskedOcclusionCulling\MaskedOcclusionCulling.h"

DepthBufferMaskedRasterizerAVXST::DepthBufferMaskedRasterizerAVXST(MaskedOcclusionCulling *moc)
	: DepthBufferRasterizerAVX()
{
	mMaskedOcclusionCulling = moc;

	memset(mNumRasterizedTris, 0, sizeof(UINT) * 2 * NUM_TILES);
}

DepthBufferMaskedRasterizerAVXST::~DepthBufferMaskedRasterizerAVXST()
{
}

//------------------------------------------------------------------------------
// * Determine if the occludee model is inside view frustum
// * Transform the occluder models on the CPU
// * Bin the occluder triangles into tiles that the frame buffer is divided into
// * Rasterize the occluder triangles to the CPU depth buffer
//-------------------------------------------------------------------------------
void DepthBufferMaskedRasterizerAVXST::TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx)
{
	// The MaskedOcclusionCulling library assumes transformed vertices in clip space.
	// We flip the y component becuase this sample uses clockwise winding as front facing.
	static const float4x4 viewportMatrix(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
	_mm_setcsr(_mm_getcsr() | 0x8040);

	QueryPerformanceCounter(&mStartTime[idx]);
	mpCamera[idx] = pCamera;
	
	BoxTestSetupSSE setup;
	setup.Init(mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrix, pCamera, mOccluderSizeThreshold);

	if(mEnableFCulling)
	{
		for(UINT i = 0; i < mNumModels1; i++)
		{
			mpTransformedModels1[i].InsideViewFrustum(setup, idx);
		}
	}
	else
	{
		for(UINT i = 0; i < mNumModels1; i++)
		{
			mpTransformedModels1[i].TooSmall(setup, idx);
		}
	}

	mMaskedOcclusionCulling->ClearBuffer();
	ActiveModels(idx);
	TransformAndRasterizeMeshes(idx);

	QueryPerformanceCounter(&mStopTime[idx][0]);
	mRasterizeTime[mTimeCounter++] = ((double)(mStopTime[idx][0].QuadPart - mStartTime[idx].QuadPart)) / ((double)glFrequency.QuadPart);
	mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;
}

void DepthBufferMaskedRasterizerAVXST::ActiveModels(UINT idx)
{
	ResetActive(idx);
	for (UINT i = 0; i < mNumModels1; i++)
	{
		if(mpTransformedModels1[i].IsRasterized2DB(idx))
		{
			Activate(i, idx);
		}
	}
}


//-------------------------------------------------
// Bins the transformed triangles into tiles
//-------------------------------------------------
void DepthBufferMaskedRasterizerAVXST::TransformAndRasterizeMeshes(UINT idx)
{
	// Sort models roughly from front to back
	std::sort(mpModelIndexA[idx], mpModelIndexA[idx] + mNumModelsA[idx], [=](int a, int b) { return mpTransformedModels1[a].GetDepth() < mpTransformedModels1[b].GetDepth(); });

	for (UINT active = 0; active < mNumModelsA[idx]; active++)
	{
		UINT ss = mpModelIndexA[idx][active];
		mpTransformedModels1[ss].TransformAndRasterizeTrianglesST(mMaskedOcclusionCulling, idx);
	}
}

void DepthBufferMaskedRasterizerAVXST::ComputeR2DBTime(UINT idx)
{
}