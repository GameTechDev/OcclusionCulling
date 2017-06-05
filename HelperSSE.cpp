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

#include "HelperSSE.h"
#include "CPUTCamera.h"

HelperSSE::HelperSSE()
{
}

HelperSSE::~HelperSSE()
{
}

__m128 HelperSSE::TransformCoords(const __m128 *v, __m128 *m)
{
	__m128 vResult = _mm_shuffle_ps(*v, *v, _MM_SHUFFLE(0,0,0,0));
    vResult = _mm_mul_ps(vResult, m[0]);

    __m128 vTemp = _mm_shuffle_ps(*v, *v, _MM_SHUFFLE(1,1,1,1));
    vTemp = _mm_mul_ps(vTemp, m[1]);
    
	vResult = _mm_add_ps(vResult, vTemp);
    vTemp = _mm_shuffle_ps(*v, *v, _MM_SHUFFLE(2,2,2,2));
    
	vTemp = _mm_mul_ps(vTemp, m[2]);
    vResult = _mm_add_ps(vResult, vTemp);
    
	vResult = _mm_add_ps(vResult, m[3]);
	return vResult;
}


void HelperSSE::MatrixMultiply(const __m128 *m1, const __m128 *m2, __m128 *result)
{
	__m128 X, Y, Z, W;
	float *mat = (float*)m1;
	 

	X = _mm_set1_ps(*(mat + 0));
	Y = _mm_set1_ps(*(mat + 1));
	Z = _mm_set1_ps(*(mat + 2));
	W = _mm_set1_ps(*(mat + 3));

	result[0] = _mm_mul_ps(X, m2[0]);
	result[0] = _mm_add_ps(result[0], _mm_mul_ps(Y, m2[1]));
	result[0] = _mm_add_ps(result[0], _mm_mul_ps(Z, m2[2]));
	result[0] = _mm_add_ps(result[0], _mm_mul_ps(W, m2[3]));

	X = _mm_set1_ps(*(mat + 4));
	Y = _mm_set1_ps(*(mat + 5));
	Z = _mm_set1_ps(*(mat + 6));
	W = _mm_set1_ps(*(mat + 7));

	result[1] = _mm_mul_ps(X, m2[0]);
	result[1] = _mm_add_ps(result[1], _mm_mul_ps(Y, m2[1]));
	result[1] = _mm_add_ps(result[1], _mm_mul_ps(Z, m2[2]));
	result[1] = _mm_add_ps(result[1], _mm_mul_ps(W, m2[3]));

	X = _mm_set1_ps(*(mat + 8));
	Y = _mm_set1_ps(*(mat + 9));
	Z = _mm_set1_ps(*(mat + 10));
	W = _mm_set1_ps(*(mat + 11));

	result[2] = _mm_mul_ps(X, m2[0]);
	result[2] = _mm_add_ps(result[2], _mm_mul_ps(Y, m2[1]));
	result[2] = _mm_add_ps(result[2], _mm_mul_ps(Z, m2[2]));
	result[2] = _mm_add_ps(result[2], _mm_mul_ps(W, m2[3]));
	
	X = _mm_set1_ps(*(mat + 12));
	Y = _mm_set1_ps(*(mat + 13));
	Z = _mm_set1_ps(*(mat + 14));
	W = _mm_set1_ps(*(mat + 15));

	result[3] = _mm_mul_ps(X, m2[0]);
	result[3] = _mm_add_ps(result[3], _mm_mul_ps(Y, m2[1]));
	result[3] = _mm_add_ps(result[3], _mm_mul_ps(Z, m2[2]));
	result[3] = _mm_add_ps(result[3], _mm_mul_ps(W, m2[3]));
	
}

void BoxTestSetupSSE::Init(const __m128 viewMatrix[4], const __m128 projMatrix[4], const float4x4 &viewportMatrix, CPUTCamera *pCamera, float occludeeSizeThreshold)
{
	__m128 viewPortMatrix[4];
	viewPortMatrix[0] = _mm_loadu_ps((float*)&viewportMatrix.r0);
	viewPortMatrix[1] = _mm_loadu_ps((float*)&viewportMatrix.r1);
	viewPortMatrix[2] = _mm_loadu_ps((float*)&viewportMatrix.r2);
	viewPortMatrix[3] = _mm_loadu_ps((float*)&viewportMatrix.r3);

	MatrixMultiply(viewMatrix, projMatrix, mViewProjViewport);
	MatrixMultiply(mViewProjViewport, viewPortMatrix, mViewProjViewport);

	mpCamera = pCamera;

	float fov = pCamera->GetFov();
	float tanOfHalfFov = tanf(fov * 0.5f);
	radiusThreshold = occludeeSizeThreshold * occludeeSizeThreshold * tanOfHalfFov;
} 