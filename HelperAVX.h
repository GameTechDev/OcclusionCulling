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

#include "immintrin.h"
#include <intrin.h>
#include "HelperSSE.h"
#ifndef HELPERAVX_H
#define HELPERAVX_H

// Find index of least-significant set bit in mask and clear it (mask must be nonzero)
//static int FindClearLSB(unsigned int *mask)
//{
//	unsigned long idx;
//	_BitScanForward(&idx, *mask);
//	*mask &= *mask - 1;
//	return idx;
//}

class HelperAVX : public HelperSSE
{
	public:
		HelperAVX();
		~HelperAVX();

	protected:
		struct vFloat4
		{
			__m128 X;
			__m128 Y;
			__m128 Z;
			__m128 W;
		};

		struct vFxPt4
		{
			__m128i X;
			__m128i Y;
			__m128i Z;
			__m128i W;
		};

		__m128 TransformCoords(const __m128 *v, __m128 *m);
		void MatrixMultiply(const __m128 *m1, const __m128 *m2, __m128 *result);

		__forceinline __m256i Min(const __m256i &v0, const __m256i &v1)
		{
			__m256i tmp;
			tmp = _mm256_min_epi32(v0, v1);
			return tmp;
		}

		__forceinline __m256i Max(const __m256i &v0, const __m256i &v1)
		{
			__m256i tmp;
			tmp = _mm256_max_epi32(v0, v1);
			return tmp;
		}
};

//class CPUTCamera;
//struct float4x4; 
//
//struct BoxTestSetupAVX : public HelperAVX
//{
//	__m128 mViewProjViewport[4];
//	CPUTCamera *mpCamera;
//	float radiusThreshold;
//
//	void Init(const __m128 viewMatrix[4], const __m128 projMatrix[4], const float4x4 &viewportMatix, CPUTCamera *pCamera, float sizeThreshold);
//}; 

#endif 