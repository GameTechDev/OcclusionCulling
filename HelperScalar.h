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

#ifndef HELPERSCALAR_H
#define HELPERSCALAR_H

#include "CPUTMath.h"

class HelperScalar
{
	public:
		HelperScalar();
		~HelperScalar();

	protected:
		struct int4
		{
			int x, y, z, w;

			int4(const float4& f4)
			{
				x = (int)(f4.x + 0.5);
				y = (int)(f4.y + 0.5);
				z = (int)(f4.z + 0.5);
				w = (int)(f4.w + 0.5);
			}
		};

		float4 TransformCoords(const float3& v, const float4x4& m);
		float4 TransformCoords(const float4& v, const float4x4& m);
};


class CPUTCamera;
struct float4x4; 

struct BoxTestSetupScalar : public HelperScalar
{
	float4x4 mViewProjViewport;
	CPUTCamera *mpCamera;
	float radiusThreshold;

	void Init(const float4x4 &viewMatrix, const float4x4 &projMatrix, const float4x4 &viewportMatix, CPUTCamera *pCamera, float sizeThreshold);
}; 

#endif