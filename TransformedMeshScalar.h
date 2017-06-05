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

#ifndef TRANSFORMEDMESHSCALAR_H
#define TRANSFORMEDMESHSCALAR_H

#include "CPUT_DX11.h"
#include "Constants.h"
#include "HelperScalar.h"

class TransformedMeshScalar : public HelperScalar
{
	public:
		TransformedMeshScalar();
		~TransformedMeshScalar();
		void Initialize(CPUTMeshDX11 *pMesh);
		void TransformVertices(const float4x4& cumulativeMatrix, 
							   UINT start, 
							   UINT end,
							   UINT idx);

		void BinTransformedTrianglesST(UINT taskId,
									   UINT modelId,
									   UINT meshId,
									   UINT start,
									   UINT end,
									   UINT* pBin,
									   USHORT* pBinModel,
									   USHORT* pBinMesh,
									   USHORT* pNumTrisInBin,
									   UINT idx);

		void BinTransformedTrianglesMT(UINT taskId,
									   UINT modelId,
									   UINT meshId,
									   UINT start,
									   UINT end,
									   UINT* pBin,
									   USHORT* pBinModel,
									   USHORT* pBinMesh,
									   USHORT* pNumTrisInBin,
									   UINT idx);

		void GetOneTriangleData(float* xformedPos, UINT triId, UINT idx);

		inline UINT GetNumTriangles() {return mNumTriangles;}
		inline UINT GetNumVertices() {return mNumVertices;}
		inline void SetXformedPos(float4 *pXformedPos0, float4 *pXformedPos1)
		{
			mpXformedPos[0] = pXformedPos0;
			mpXformedPos[1] = pXformedPos1;
		}
	
	private:
		UINT mNumVertices;
		UINT mNumIndices;
		UINT mNumTriangles;
		Vertex *mpVertices;
		UINT *mpIndices;
		float4 *mpXformedPos[2]; 
		
		void Gather(float4 pOut[3], UINT triId, UINT idx);
};


#endif