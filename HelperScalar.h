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