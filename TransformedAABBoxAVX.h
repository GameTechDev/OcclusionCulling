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


#ifndef TRANSFORMEDAABBOXAVX_H
#define TRANSFORMEDAABBOXAVX_H

#include "CPUT_DX11.h"
#include "Constants.h"
#include "HelperAVX.h"

#include "TransformedAABBoxSSE.h"
//enum PreTestResult
//{
//	ePT_INVISIBLE,
//	ePT_VISIBLE,
//	ePT_UNSURE,
//};

class MaskedOcclusionCulling;
class CullingThreadpool;

class TransformedAABBoxAVX : public HelperAVX
{
	public:
		void CreateAABBVertexIndexList(CPUTModelDX11 *pModel);
		bool IsInsideViewFrustum(CPUTCamera *pCamera);
		PreTestResult TransformAndPreTestAABBox(__m128 xformedPos[], const __m128 cumulativeMatrix[4], const float *pDepthSummary);
		PreTestResult TransformAndPreTestAABBox(__m128 xformedPos[], const __m128 cumulativeMatrix[4], MaskedOcclusionCulling *moc);
		bool RasterizeAndDepthTestAABBox(UINT *pRenderTargetPixels, const __m128 pXformedPos[], UINT idx);
		bool RasterizeAndDepthTestAABBox(MaskedOcclusionCulling *moc, const __m128 pXformedPos[]);

		bool IsTooSmall(const BoxTestSetupSSE &setup, __m128 cumulativeMatrix[4]);
		
	private:
		CPUTModelDX11 *mpCPUTModel;
		float4x4 mWorldMatrix;
		
		float3 mBBCenter;
		float3 mBBHalf;
		float  mRadiusSq;
		
		float3 mBBCenterWS;
		float3 mBBHalfWS;

		void Gather(vFloat4 pOut[3], UINT triId, const __m128 xformedPos[], UINT idx);
};


#endif // TRANSFORMEDAABBOXAVX_H