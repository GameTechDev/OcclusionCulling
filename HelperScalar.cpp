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

#include "HelperScalar.h"
#include "CPUTCamera.h"

HelperScalar::HelperScalar()
{
}

HelperScalar::~HelperScalar()
{
}

float4 HelperScalar::TransformCoords(const float4 &v, const float4x4 &m)
{
	float4 result;

	result.x = v.x * m.r0.x + v.y * m.r1.x  + v.z * m.r2.x + v.w * m.r3.x;
	result.y = v.x * m.r0.y + v.y * m.r1.y  + v.z * m.r2.y + v.w * m.r3.y;
	result.z = v.x * m.r0.z + v.y * m.r1.z  + v.z * m.r2.z + v.w * m.r3.z;
	result.w = v.x * m.r0.w + v.y * m.r1.w  + v.z * m.r2.w + v.w * m.r3.w;

	return result;
}

float4 HelperScalar::TransformCoords(const float3 &v, const float4x4 &m)
{
	float4 result;
	
	result.x = v.x * m.r0.x + v.y * m.r1.x  + v.z * m.r2.x + m.r3.x;
	result.y = v.x * m.r0.y + v.y * m.r1.y  + v.z * m.r2.y + m.r3.y;
	result.z = v.x * m.r0.z + v.y * m.r1.z  + v.z * m.r2.z + m.r3.z;
	result.w = v.x * m.r0.w + v.y * m.r1.w  + v.z * m.r2.w + m.r3.w;

	return result;
}


void BoxTestSetupScalar::Init(const float4x4 &viewMatrix, const float4x4 &projMatrix, const float4x4 &viewportMatrix, CPUTCamera *pCamera, float occludeeSizeThreshold)
{
	mViewProjViewport = viewMatrix * projMatrix;
	mViewProjViewport = mViewProjViewport * viewportMatrix;

	mpCamera = pCamera;

	float fov = pCamera->GetFov();
	float tanOfHalfFov = tanf(fov * 0.5f);
	radiusThreshold = occludeeSizeThreshold * occludeeSizeThreshold * tanOfHalfFov;
}