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

#include "AABBoxMaskedRasterizerAVXST.h"
#include "MaskedOcclusionCulling\MaskedOcclusionCulling.h"

AABBoxMaskedRasterizerAVXST::AABBoxMaskedRasterizerAVXST(MaskedOcclusionCulling *moc)
	: AABBoxRasterizerAVX()
{
	mMaskedOcclusionCulling = moc;
}

AABBoxMaskedRasterizerAVXST::~AABBoxMaskedRasterizerAVXST()
{

}

//------------------------------------------------------------------------------
// For each occludee model
// * Determine if the occludee model AABox is within the viewing frustum 
// * Transform the AABBox to screen space
// * Rasterize the triangles that make up the AABBox
// * Depth test the raterized triangles against the CPU rasterized depth buffer
//-----------------------------------------------------------------------------
void AABBoxMaskedRasterizerAVXST::TransformAABBoxAndDepthTest(CPUTCamera *pCamera, UINT idx)
{
	// Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
	_mm_setcsr(_mm_getcsr() | 0x8040);

	QueryPerformanceCounter(&mStartTime[idx][0]);
	mpCamera[idx] = pCamera;

	if(mEnableFCulling)
	{
		CalcInsideFrustum(&mpCamera[idx]->mFrustum , 0, mNumModels, idx);
	}

	BoxTestSetupSSE setup;
	setup.Init(mViewMatrix[idx], mProjMatrix[idx], viewportMatrixMaskedOcclusionCulling, mpCamera[idx], mOccludeeSizeThreshold);

	__m128 xformedPos[AABB_VERTICES];
	__m128 cumulativeMatrix[4];

	for(UINT i = 0; i < mNumModels; i++)
	{
		mpVisible[idx][i] = false;
		
		if(mpInsideFrustum[idx][i] && !mpTransformedAABBox[i].IsTooSmall(setup, cumulativeMatrix))
		{
			PreTestResult res = mpTransformedAABBox[i].TransformAndPreTestAABBox(xformedPos, cumulativeMatrix, mMaskedOcclusionCulling);
			//if(res == ePT_UNSURE)
			//{
			//	mpVisible[idx][i] = mpTransformedAABBox[i].RasterizeAndDepthTestAABBox(mMaskedOcclusionCulling, xformedPos);
			//}
			//else
			//{
				mpVisible[idx][i] = (res != ePT_INVISIBLE);
			//}
		}		
	}


	QueryPerformanceCounter(&mStopTime[idx][0]);
	mDepthTestTime[mTimeCounter++] = ((double)(mStopTime[idx][0].QuadPart - mStartTime[idx][0].QuadPart)) / ((double)glFrequency.QuadPart);
	mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;
}

void AABBoxMaskedRasterizerAVXST::WaitForTaskToFinish(UINT idx)
{
}

void AABBoxMaskedRasterizerAVXST::ReleaseTaskHandles(UINT idx)
{
}