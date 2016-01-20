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

#include "immintrin.h"
#include <intrin.h>
#ifndef HELPERSSE_H
#define HELPERSSE_H

// Find index of least-significant set bit in mask and clear it (mask must be nonzero)
static int FindClearLSB(unsigned int *mask)
{
	unsigned long idx;
	_BitScanForward(&idx, *mask);
	*mask &= *mask - 1;
	return idx;
}

class HelperSSE
{
	public:
		HelperSSE();
		~HelperSSE();

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

		__forceinline __m128i Min(const __m128i &v0, const __m128i &v1)
		{
			__m128i tmp;
			tmp = _mm_min_epi32(v0, v1);
			return tmp;
		}

		__forceinline __m128i Max(const __m128i &v0, const __m128i &v1)
		{
			__m128i tmp;
			tmp = _mm_max_epi32(v0, v1);
			return tmp;
		}
};

class CPUTCamera;
struct float4x4; 

struct BoxTestSetupSSE : public HelperSSE
{
	__m128 mViewProjViewport[4];
	CPUTCamera *mpCamera;
	float radiusThreshold;

	void Init(const __m128 viewMatrix[4], const __m128 projMatrix[4], const float4x4 &viewportMatix, CPUTCamera *pCamera, float sizeThreshold);
}; 

#endif 