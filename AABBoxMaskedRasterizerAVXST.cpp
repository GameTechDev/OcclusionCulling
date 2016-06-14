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

	// The MaskedOcclusionCulling library assumes transformed vertices in clip space.
	// We flip the y component becuase this sample uses clockwise winding as front facing.
	static const float4x4 viewportMatrix(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	BoxTestSetupSSE setup;
	setup.Init(mViewMatrix[idx], mProjMatrix[idx], viewportMatrix, mpCamera[idx], mOccludeeSizeThreshold);

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