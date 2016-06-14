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

#include "TransformedMeshSSE.h"
#include "DepthBufferRasterizerSSE.h"
#include "MaskedOcclusionCulling\MaskedOcclusionCulling.h"

TransformedMeshSSE::TransformedMeshSSE()
	: mNumVertices(0),
	  mNumIndices(0),
	  mNumTriangles(0),
	  mpVertices(NULL),
	  mpIndices(NULL)
{
	mpXformedPos[0] = mpXformedPos[1] = NULL;
}

TransformedMeshSSE::~TransformedMeshSSE()
{

}

void TransformedMeshSSE::Initialize(CPUTMeshDX11* pMesh)
{
	mNumVertices = pMesh->GetDepthVertexCount();
	mNumIndices  = pMesh->GetIndexCount();
	mNumTriangles = pMesh->GetTriangleCount();
	mpVertices   = pMesh->GetDepthVertices();
	mpIndices    = pMesh->GetDepthIndices();
}

inline __m128d& operator+=(__m128d& v1, const __m128d& v2) {
	return (v1 = _mm_add_pd(v1, v2));
}

inline __m128d& operator-=(__m128d& v1, const __m128d& v2) {
	return (v1 = _mm_sub_pd(v1, v2));
}

inline __m128d& operator*=(__m128d& v1, const __m128d& v2) {
	return (v1 = _mm_mul_pd(v1, v2));
}

inline __m128d& operator/=(__m128d& v1, const __m128d& v2) {
	return (v1 = _mm_div_pd(v1, v2));
}

inline __m128d operator+(const __m128d& v1, const __m128d& v2) {
	return _mm_add_pd(v1, v2);
}

inline __m128d operator-(const __m128d& v1, const __m128d& v2) {
	return _mm_sub_pd(v1, v2);
}

inline __m128d operator*(const __m128d& v1, const __m128d& v2) {
	return _mm_mul_pd(v1, v2);
}

inline __m128d operator/(const __m128d& v1, const __m128d& v2) {
	return _mm_div_pd(v1, v2);
}

inline __m128& operator+=(__m128& v1, const __m128& v2) {
	return (v1 = _mm_add_ps(v1, v2));
}

inline __m128& operator-=(__m128& v1, const __m128& v2) {
	return (v1 = _mm_sub_ps(v1, v2));
}

inline __m128& operator*=(__m128& v1, const __m128& v2) {
	return (v1 = _mm_mul_ps(v1, v2));
}

inline __m128& operator/=(__m128& v1, const __m128& v2) {
	return (v1 = _mm_div_ps(v1, v2));
}

inline __m128 operator+(const __m128& v1, const __m128& v2) {
	return _mm_add_ps(v1, v2);
}

inline __m128 operator-(const __m128& v1, const __m128& v2) {
	return _mm_sub_ps(v1, v2);
}

inline __m128 operator*(const __m128& v1, const __m128& v2) {
	return _mm_mul_ps(v1, v2);
}

inline __m128 operator/(const __m128& v1, const __m128& v2) {
	return _mm_div_ps(v1, v2);
}

//-------------------------------------------------------------------
// Trasforms the occluder vertices to screen space once every frame
//-------------------------------------------------------------------
void TransformedMeshSSE::TransformVertices(__m128 *cumulativeMatrix, 
										   UINT start, 
										   UINT end,
										   UINT idx)
{
	__m128 row0 = cumulativeMatrix[0];
	__m128 row1 = cumulativeMatrix[1];
	__m128 row2 = cumulativeMatrix[2];
	__m128 row3 = cumulativeMatrix[3];
	Vertex * const inPos = mpVertices;
	
	for(UINT i = start; i <= end; i++)
	{
		__m128 xform = row3;
		xform += row0 * _mm_load1_ps(&inPos[i].pos.x);
		xform += row1 * _mm_load1_ps(&inPos[i].pos.y);
		xform += row2 * _mm_load1_ps(&inPos[i].pos.z);

		__m128 vertZ = _mm_shuffle_ps(xform, xform, 0xaa);
		__m128 vertW = _mm_shuffle_ps(xform, xform, 0xff);
		__m128 projected = _mm_div_ps(xform, vertW);

		//set to all 0s if clipped by near clip plane
		__m128 noNearClip = _mm_cmple_ps(vertZ, vertW);
		mpXformedPos[idx][i] = _mm_and_ps(projected, noNearClip);
	}
}

void TransformedMeshSSE::Gather(vFloat4 pOut[3], UINT triId, UINT numLanes, UINT idx)
{
	const UINT *pInd0 = &mpIndices[triId * 3];
	const UINT *pInd1 = pInd0 + (numLanes > 1 ? 3 : 0);
	const UINT *pInd2 = pInd0 + (numLanes > 2 ? 6 : 0);
	const UINT *pInd3 = pInd0 + (numLanes > 3 ? 9 : 0);

	for(UINT i = 0; i < 3; i++)
	{
		__m128 v0 = mpXformedPos[idx][pInd0[i]];	// x0 y0 z0 w0
		__m128 v1 = mpXformedPos[idx][pInd1[i]];	// x1 y1 z1 w1
		__m128 v2 = mpXformedPos[idx][pInd2[i]];	// x2 y2 z2 w2
		__m128 v3 = mpXformedPos[idx][pInd3[i]];	// x3 y3 z3 w3
		_MM_TRANSPOSE4_PS(v0, v1, v2, v3);
		pOut[i].X = v0;
		pOut[i].Y = v1;
		pOut[i].Z = v2;
		pOut[i].W = v3;
	}
}

//--------------------------------------------------------------------------------
// Bin the screen space transformed triangles into tiles. For single threaded version
//--------------------------------------------------------------------------------
void TransformedMeshSSE::BinTransformedTrianglesST(UINT taskId,
												   UINT modelId,
												   UINT meshId,
												   UINT start,
												   UINT end,
												   BinTriangle *pBin,
												   USHORT* pNumTrisInBin,
												   UINT idx)
{
	int numLanes = SSE;
	int laneMask = (1 << numLanes) - 1; 
	// working on 4 triangles at a time
	for(UINT index = start; index <= end; index += SSE)
	{
		if(index + SSE > end)
		{
			numLanes = end - index + 1;
			laneMask = (1 << numLanes) - 1; 
		}
		
		// storing x,y,z,w for the 3 vertices of 4 triangles = 4*3*4 = 48
		vFloat4 xformedPos[3];		
		Gather(xformedPos, index, numLanes, idx);
		
		// Convert screen space position to Fixed pt 
		__m128i fxPtX[3], fxPtY[3];
		__m128i vXY[3];
		__m128 vZ[3];
		for(int i = 0; i < 3; i++)
		{
			fxPtX[i] = _mm_cvtps_epi32(xformedPos[i].X);
			fxPtY[i] = _mm_cvtps_epi32(xformedPos[i].Y);

			__m128i inter0 = _mm_unpacklo_epi32(fxPtX[i], fxPtY[i]);
			__m128i inter1 = _mm_unpackhi_epi32(fxPtX[i], fxPtY[i]);

			vXY[i] = _mm_packs_epi32(inter0, inter1);
			vZ[i] = xformedPos[i].Z;
		}

		// Compute triangle area
		__m128i triArea1 = _mm_sub_epi32(fxPtX[1], fxPtX[0]);
		triArea1 = _mm_mullo_epi32(triArea1, _mm_sub_epi32(fxPtY[2], fxPtY[0]));

		__m128i triArea2 = _mm_sub_epi32(fxPtX[0], fxPtX[2]);
		triArea2 = _mm_mullo_epi32(triArea2, _mm_sub_epi32(fxPtY[0], fxPtY[1]));

		__m128i triArea = _mm_sub_epi32(triArea1, triArea2);
		__m128 oneOverTriArea = _mm_rcp_ps(_mm_cvtepi32_ps(triArea));
		
		// Z setup
		vZ[1] = _mm_mul_ps(_mm_sub_ps(vZ[1], vZ[0]), oneOverTriArea);
		vZ[2] = _mm_mul_ps(_mm_sub_ps(vZ[2], vZ[0]), oneOverTriArea);

		// Find bounding box for screen space triangle in terms of pixels
		__m128i vStartX = Max(Min(Min(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(0));
		__m128i vEndX   = Min(Max(Max(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(SCREENW - 1));

        __m128i vStartY = Max(Min(Min(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(0));
        __m128i vEndY   = Min(Max(Max(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(SCREENH - 1));

		//Figure out which lanes are active
		__m128i front = _mm_cmpgt_epi32(triArea, _mm_setzero_si128());
		__m128i nonEmptyX = _mm_cmpgt_epi32(vEndX, vStartX);
		__m128i nonEmptyY = _mm_cmpgt_epi32(vEndY, vStartY);
		__m128 accept1 = _mm_castsi128_ps(_mm_and_si128(_mm_and_si128(front, nonEmptyX), nonEmptyY));

		// All verts must be inside the near clip volume
		__m128 W0 = _mm_cmpgt_ps(xformedPos[0].W, _mm_setzero_ps());
		__m128 W1 = _mm_cmpgt_ps(xformedPos[1].W, _mm_setzero_ps());
		__m128 W2 = _mm_cmpgt_ps(xformedPos[2].W, _mm_setzero_ps());

		__m128 accept = _mm_and_ps(_mm_and_ps(accept1, W0), _mm_and_ps(W1, W2));
		unsigned int triMask = _mm_movemask_ps(accept) & laneMask; 
		
		while(triMask)
		{
			int i = FindClearLSB(&triMask);
			// Convert bounding box in terms of pixels to bounding box in terms of tiles
			int startX = max(vStartX.m128i_i32[i]/TILE_WIDTH_IN_PIXELS, 0);
			int endX   = min(vEndX.m128i_i32[i]/TILE_WIDTH_IN_PIXELS, SCREENW_IN_TILES-1);

			int startY = max(vStartY.m128i_i32[i]/TILE_HEIGHT_IN_PIXELS, 0);
			int endY   = min(vEndY.m128i_i32[i]/TILE_HEIGHT_IN_PIXELS, SCREENH_IN_TILES-1);

			// Add triangle to the tiles or bins that the bounding box covers
			int row, col;
			for(row = startY; row <= endY; row++)
			{
				int offset1 = YOFFSET1_ST * row;
				int offset2 = YOFFSET2_ST * row;
				for(col = startX; col <= endX; col++)
				{
					int idx1 = offset1 + (XOFFSET1_ST * col) + taskId;
					int idx2 = offset2 + (XOFFSET2_ST * col) + (taskId * MAX_TRIS_IN_BIN_ST) + pNumTrisInBin[idx1];
					BinTriangle *pTri = pBin + idx2;
					pTri->vert[0].xy = vXY[0].m128i_i32[i];
					pTri->vert[1].xy = vXY[1].m128i_i32[i];
					pTri->vert[2].xy = vXY[2].m128i_i32[i];
					pTri->Z[0] = vZ[0].m128_f32[i];
					pTri->Z[1] = vZ[1].m128_f32[i];
					pTri->Z[2] = vZ[2].m128_f32[i];
					pNumTrisInBin[idx1] += 1;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------
// Bin the screen space transformed triangles into tiles. For multi threaded version
//--------------------------------------------------------------------------------
void TransformedMeshSSE::BinTransformedTrianglesMT(UINT taskId,
												   UINT modelId,
												   UINT meshId,
												   UINT start,
												   UINT end,
												   BinTriangle *pBin,
												   USHORT* pNumTrisInBin,
												   UINT idx)
{
	int numLanes = SSE;
	int laneMask = (1 << numLanes) - 1; 
	// working on 4 triangles at a time
	for(UINT index = start; index <= end; index += SSE)
	{
		if(index + SSE > end)
		{
			numLanes = end - index + 1;
			laneMask = (1 << numLanes) - 1; 
		}
		
		// storing x,y,z,w for the 3 vertices of 4 triangles = 4*3*4 = 48
		vFloat4 xformedPos[3];
		Gather(xformedPos, index, numLanes, idx);
			
		// Convert screen space position to Fixed pt
		__m128i fxPtX[3], fxPtY[3];	
		__m128i vXY[3];
		__m128 vZ[3];
		for(int i = 0; i < 3; i++)
		{
			fxPtX[i] = _mm_cvtps_epi32(xformedPos[i].X);
			fxPtY[i] = _mm_cvtps_epi32(xformedPos[i].Y);

			__m128i inter0 = _mm_unpacklo_epi32(fxPtX[i], fxPtY[i]);
			__m128i inter1 = _mm_unpackhi_epi32(fxPtX[i], fxPtY[i]);

			vXY[i] = _mm_packs_epi32(inter0, inter1);
			vZ[i] = xformedPos[i].Z;
		}

		__m128i triArea1 = _mm_sub_epi32(fxPtX[1], fxPtX[0]);
		triArea1 = _mm_mullo_epi32(triArea1, _mm_sub_epi32(fxPtY[2], fxPtY[0]));

		__m128i triArea2 = _mm_sub_epi32(fxPtX[0], fxPtX[2]);
		triArea2 = _mm_mullo_epi32(triArea2, _mm_sub_epi32(fxPtY[0], fxPtY[1]));

		__m128i triArea = _mm_sub_epi32(triArea1, triArea2);
		__m128 oneOverTriArea = _mm_div_ps(_mm_set1_ps(1.0f), _mm_cvtepi32_ps(triArea));
		
		// Z setup
		vZ[1] = _mm_mul_ps(_mm_sub_ps(vZ[1], vZ[0]), oneOverTriArea);
		vZ[2] = _mm_mul_ps(_mm_sub_ps(vZ[2], vZ[0]), oneOverTriArea);

		// Find bounding box for screen space triangle in terms of pixels
		__m128i vStartX = Max(Min(Min(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(0));
		__m128i vEndX   = Min(Max(Max(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(SCREENW - 1));

        __m128i vStartY = Max(Min(Min(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(0));
        __m128i vEndY   = Min(Max(Max(fxPtY[0], fxPtY[1]), fxPtY[2]),  _mm_set1_epi32(SCREENH -1));

		//Figure out which lanes are active
		__m128i front = _mm_cmpgt_epi32(triArea, _mm_setzero_si128());
		__m128i nonEmptyX = _mm_cmpgt_epi32(vEndX, vStartX);
		__m128i nonEmptyY = _mm_cmpgt_epi32(vEndY, vStartY);
		__m128 accept1 = _mm_castsi128_ps(_mm_and_si128(_mm_and_si128(front, nonEmptyX), nonEmptyY));

		// All verts must be inside the near clip volume
		__m128 W0 = _mm_cmpgt_ps(xformedPos[0].W, _mm_setzero_ps());
		__m128 W1 = _mm_cmpgt_ps(xformedPos[1].W, _mm_setzero_ps());
		__m128 W2 = _mm_cmpgt_ps(xformedPos[2].W, _mm_setzero_ps());

		__m128 accept = _mm_and_ps(_mm_and_ps(accept1, W0), _mm_and_ps(W1, W2));
		unsigned int triMask = _mm_movemask_ps(accept) & laneMask; 
			
		while(triMask)
		{
			int i = FindClearLSB(&triMask);
				
			// Convert bounding box in terms of pixels to bounding box in terms of tiles
			int startX = max(vStartX.m128i_i32[i]/TILE_WIDTH_IN_PIXELS, 0);
			int endX   = min(vEndX.m128i_i32[i]/TILE_WIDTH_IN_PIXELS, SCREENW_IN_TILES-1);
			int startY = max(vStartY.m128i_i32[i]/TILE_HEIGHT_IN_PIXELS, 0);
			int endY   = min(vEndY.m128i_i32[i]/TILE_HEIGHT_IN_PIXELS, SCREENH_IN_TILES-1);

			// Add triangle to the tiles or bins that the bounding box covers
			int row, col;
			for(row = startY; row <= endY; row++)
			{
				int offset1 = YOFFSET1_MT * row;
				int offset2 = YOFFSET2_MT * row;
				for(col = startX; col <= endX; col++)
				{
					int idx1 = offset1 + (XOFFSET1_MT * col) + (TOFFSET1_MT * taskId);
					int idx2 = offset2 + (XOFFSET2_MT * col) + (taskId * MAX_TRIS_IN_BIN_MT) + pNumTrisInBin[idx1];
					BinTriangle *pTri = pBin + idx2;
					pTri->vert[0].xy = vXY[0].m128i_i32[i];
					pTri->vert[1].xy = vXY[1].m128i_i32[i];
					pTri->vert[2].xy = vXY[2].m128i_i32[i];
					pTri->Z[0] = vZ[0].m128_f32[i];
					pTri->Z[1] = vZ[1].m128_f32[i];
					pTri->Z[2] = vZ[2].m128_f32[i];
					pNumTrisInBin[idx1] += 1;
				}
			}
		}
	}
}

void TransformedMeshSSE::TransformAndRasterizeTrianglesST(__m128 *cumulativeMatrix, MaskedOcclusionCulling *moc, UINT idx)
{
	memset(mpXformedPos[idx], 0, sizeof(__m128)*GetNumVertices());
	MaskedOcclusionCulling::TransformVertices((float*)cumulativeMatrix, (float*)mpVertices, (float*)mpXformedPos[idx], GetNumVertices());
	moc->RenderTriangles((float*)mpXformedPos[idx], mpIndices, GetNumTriangles(), MaskedOcclusionCulling::CLIP_PLANE_ALL, nullptr);
}