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