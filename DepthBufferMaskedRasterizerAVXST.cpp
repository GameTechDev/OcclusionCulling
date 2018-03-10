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
	// Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
	_mm_setcsr(_mm_getcsr() | 0x8040);

	QueryPerformanceCounter(&mStartTime[idx]);
	mpCamera[idx] = pCamera;
	
	BoxTestSetupSSE setup;
	setup.Init(mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrixMaskedOcclusionCulling, pCamera, mOccluderSizeThreshold);

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

    {
        mMaskedOcclusionCulling->ClearBuffer();
    }
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

    {
	    // Sort models roughly from front to back
	    std::sort(mpModelIndexA[idx], mpModelIndexA[idx] + mNumModelsA[idx], [=](int a, int b) { return mpTransformedModels1[a].GetDepth() < mpTransformedModels1[b].GetDepth(); });
    }

	for (UINT active = 0; active < mNumModelsA[idx]; active++)
	{
		UINT ss = mpModelIndexA[idx][active];
		mpTransformedModels1[ss].TransformAndRasterizeTrianglesST(mMaskedOcclusionCulling, idx);
	}
}

void DepthBufferMaskedRasterizerAVXST::ComputeR2DBTime(UINT idx)
{
}