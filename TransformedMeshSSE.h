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

#ifndef TRANSFORMEDMESHSSE_H
#define TRANSFORMEDMESHSSE_H

#include "CPUT_DX11.h"
#include "Constants.h"
#include "HelperSSE.h"

struct BinTriangle;

class TransformedMeshSSE : public HelperSSE
{
	public:
		TransformedMeshSSE();
		~TransformedMeshSSE();
		void Initialize(CPUTMeshDX11* pMesh);
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
		__m128 *mpXformedPos[2]; 
		
		void Gather(vFloat4 pOut[3], UINT triId, UINT numLanes, UINT idx);
};


#endif
