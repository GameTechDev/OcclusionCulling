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


#ifndef TRANSFORMEDAABBOXSCALAR_H
#define TRANSFORMEDAABBOXSCALAR_H

#include "CPUT_DX11.h"
#include "Constants.h"
#include "HelperScalar.h"

class TransformedAABBoxScalar : public HelperScalar
{
	public:
		void CreateAABBVertexIndexList(CPUTModelDX11 *pModel);
		bool IsInsideViewFrustum(CPUTCamera *pcamera);
		bool TransformAABBox(float4 xformedPos[], const float4x4 &cumulativeMatrix);
		bool RasterizeAndDepthTestAABBox(UINT *pRenderTargetPixels, const float4 pXformedPos[], UINT idx);

		bool IsTooSmall(const BoxTestSetupScalar &setup, float4x4 &cumulativeMatrix);
		
	private:
		CPUTModelDX11 *mpCPUTModel;
		float4x4 mWorldMatrix;
			
		float3  mBBCenter;
		float3  mBBHalf;
		float   mRadiusSq;

		float3  mBBCenterWS;
		float3  mBBHalfWS;

		void Gather(float4 pOut[3], UINT triId, const float4 xformedPos[]);
};


#endif // TRANSFORMEDAABBOXSCALAR_H