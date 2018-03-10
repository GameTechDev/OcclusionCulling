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

#ifndef TRANSFORMEDMESHSSE_H
#define TRANSFORMEDMESHSSE_H

#include "CPUT_DX11.h"
#include "Constants.h"
#include "HelperSSE.h"

struct BinTriangle;
class MaskedOcclusionCulling;
class CullingThreadpool;

class TransformedMeshSSE : public HelperSSE
{
	public:
		TransformedMeshSSE();
		~TransformedMeshSSE();
		void Initialize(CPUTMeshDX11* pMesh);
        void UpdateReversedWindingIndices();
		void TransformVertices(__m128 *cumulativeMatrix, 
							   UINT start, 
							   UINT end,
							   UINT idx);

		void BinTransformedTrianglesST(UINT taskId,
									   UINT modelId,
									   UINT meshId,
									   UINT start,
									   UINT end,
									   BinTriangle* pBin,
									   USHORT* pNumTrisInBin,
									   UINT idx);

		void BinTransformedTrianglesMT(UINT taskId,
									   UINT modelId,
									   UINT meshId,
									   UINT start,
									   UINT end,
									   BinTriangle* pBin,
									   USHORT* pNumTrisInBin,
									   UINT idx);

		void TransformAndRasterizeTrianglesST(__m128 *cumulativeMatrix, MaskedOcclusionCulling *moc, UINT idx);
        void TransformAndRasterizeTrianglesMT(__m128 *cumulativeMatrix, CullingThreadpool * mocThreadpool, UINT idx);

		inline UINT GetNumTriangles() {return mNumTriangles;}
		inline UINT GetNumVertices() {return mNumVertices;}
		inline void SetXformedPos(__m128 *pXformedPos0, __m128 *pXformedPos1)
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
        std::vector<UINT> mIndicesTheOtherWayAround;
		__m128 *mpXformedPos[2]; 
		
		void Gather(vFloat4 pOut[3], UINT triId, UINT numLanes, UINT idx);
};


#endif
