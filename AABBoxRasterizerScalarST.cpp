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

#include "AABBoxRasterizerScalarST.h"

AABBoxRasterizerScalarST::AABBoxRasterizerScalarST()
	: AABBoxRasterizerScalar()
{

}

AABBoxRasterizerScalarST::~AABBoxRasterizerScalarST()
{

}

//------------------------------------------------------------------------------
// For each occludee model
// * Determine if the occludee model AABox is within the viewing frustum 
// * Transform the AABBox to screen space
// * Rasterize the triangles that make up the AABBox
// * Depth test the raterized triangles against the CPU rasterized depth buffer
//-----------------------------------------------------------------------------
void AABBoxRasterizerScalarST::TransformAABBoxAndDepthTest(CPUTCamera *pCamera, UINT idx)
{
	QueryPerformanceCounter(&mStartTime[idx][0]);

	mpCamera[idx] = pCamera;
	if(mEnableFCulling)
	{
		for(UINT i = 0; i < mNumModels; i++)
		{
			mpInsideFrustum[idx][i] = mpTransformedAABBox[i].IsInsideViewFrustum(mpCamera[idx]);
		}
	}

	BoxTestSetupScalar setup;
	setup.Init(mViewMatrix[idx], mProjMatrix[idx], viewportMatrix, mpCamera[idx], mOccludeeSizeThreshold);

	float4 xformedPos[AABB_VERTICES];
	float4x4 cumulativeMatrix;

	for(UINT i = 0; i < mNumModels; i++)
	{
		mpVisible[idx][i] = false;
		
		if(mpInsideFrustum[idx][i] && !mpTransformedAABBox[i].IsTooSmall(setup, cumulativeMatrix))
		{
			if(mpTransformedAABBox[i].TransformAABBox(xformedPos, cumulativeMatrix))
			{
				mpVisible[idx][i] = mpTransformedAABBox[i].RasterizeAndDepthTestAABBox(mpRenderTargetPixels[idx], xformedPos, idx);
			}
			else
			{
				mpVisible[idx][i] = true;
			}
		}		
	}

	QueryPerformanceCounter(&mStopTime[idx][0]);
	mDepthTestTime[mTimeCounter++] = ((double)(mStopTime[idx][0].QuadPart - mStartTime[idx][0].QuadPart)) / ((double)glFrequency.QuadPart);
	mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;
}

void AABBoxRasterizerScalarST::WaitForTaskToFinish(UINT idx)
{
}

void AABBoxRasterizerScalarST::ReleaseTaskHandles(UINT idx)
{
}