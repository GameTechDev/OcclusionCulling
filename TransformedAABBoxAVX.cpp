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

#include "TransformedAABBoxAVX.h"
#include "MaskedOcclusionCulling\MaskedOcclusionCulling.h"

static const UINT sBBIndexList[AABB_INDICES] =
{
	// index for top 
	1, 3, 2,
	0, 3, 1,

	// index for bottom
	5, 7, 4,
	6, 7, 5,

	// index for left
	1, 7, 6,
	2, 7, 1,

	// index for right
	3, 5, 4,
	0, 5, 3,

	// index for back
	2, 4, 7,
	3, 4, 2,

	// index for front
	0, 6, 5,
	1, 6, 0,
};

// 0 = use min corner, 1 = use max corner
static const UINT sBBxInd[AABB_VERTICES] = { 1, 0, 0, 1, 1, 1, 0, 0 };
static const UINT sBByInd[AABB_VERTICES] = { 1, 1, 1, 1, 0, 0, 0, 0 };
static const UINT sBBzInd[AABB_VERTICES] = { 1, 1, 0, 0, 0, 1, 1, 0 };

//--------------------------------------------------------------------------
// Get the bounding box center and half vector
// Create the vertex and index list for the triangles that make up the bounding box
//--------------------------------------------------------------------------
void TransformedAABBoxAVX::CreateAABBVertexIndexList(CPUTModelDX11 *pModel)
{
	mWorldMatrix = *pModel->GetWorldMatrix();

	pModel->GetBoundsObjectSpace(&mBBCenter, &mBBHalf);
	mRadiusSq = mBBHalf.lengthSq();
	pModel->GetBoundsWorldSpace(&mBBCenterWS, &mBBHalfWS);
}

//----------------------------------------------------------------
// Determine is model is inside view frustum
//----------------------------------------------------------------
bool TransformedAABBoxAVX::IsInsideViewFrustum(CPUTCamera *pCamera)
{
	return pCamera->mFrustum.IsVisible(mBBCenterWS, mBBHalfWS);
}

//----------------------------------------------------------------------------
// Determine if the occluddee size is too small and if so avoid drawing it
//----------------------------------------------------------------------------
bool TransformedAABBoxAVX::IsTooSmall(const BoxTestSetupSSE &setup, __m128 cumulativeMatrix[4])
{
	__m128 worldMatrix[4];
	worldMatrix[0] = _mm_loadu_ps(mWorldMatrix.r0.f);
	worldMatrix[1] = _mm_loadu_ps(mWorldMatrix.r1.f);
	worldMatrix[2] = _mm_loadu_ps(mWorldMatrix.r2.f);
	worldMatrix[3] = _mm_loadu_ps(mWorldMatrix.r3.f);
	MatrixMultiply(worldMatrix, setup.mViewProjViewport, cumulativeMatrix);
	
	float w = mBBCenter.x * cumulativeMatrix[0].m128_f32[3] + 
		      mBBCenter.y * cumulativeMatrix[1].m128_f32[3] + 
			  mBBCenter.z * cumulativeMatrix[2].m128_f32[3] + 
			  cumulativeMatrix[3].m128_f32[3];

	if( w > 1.0f )
	{
		return mRadiusSq < w * setup.radiusThreshold;
	}
	return false;
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

//----------------------------------------------------------------
// Transforms the AABB vertices to screen space once every frame
// Also performs a coarse depth pre-test
//----------------------------------------------------------------
PreTestResult TransformedAABBoxAVX::TransformAndPreTestAABBox(__m128 xformedPos[], const __m128 cumulativeMatrix[4], MaskedOcclusionCulling *moc)
{
	// w ends up being garbage, but it doesn't matter - we ignore it anyway.
	__m128 vCenter = _mm_loadu_ps(&mBBCenter.x);
	__m128 vHalf = _mm_loadu_ps(&mBBHalf.x);

	__m128 vMin = _mm_sub_ps(vCenter, vHalf);
	__m128 vMax = _mm_add_ps(vCenter, vHalf);

	// transforms
	__m128 xRow[2], yRow[2], zRow[2];
	xRow[0] = _mm_shuffle_ps(vMin, vMin, 0x00) * cumulativeMatrix[0];
	xRow[1] = _mm_shuffle_ps(vMax, vMax, 0x00) * cumulativeMatrix[0];
	yRow[0] = _mm_shuffle_ps(vMin, vMin, 0x55) * cumulativeMatrix[1];
	yRow[1] = _mm_shuffle_ps(vMax, vMax, 0x55) * cumulativeMatrix[1];
	zRow[0] = _mm_shuffle_ps(vMin, vMin, 0xaa) * cumulativeMatrix[2];
	zRow[1] = _mm_shuffle_ps(vMax, vMax, 0xaa) * cumulativeMatrix[2];

	__m128 zAllIn = _mm_castsi128_ps(_mm_set1_epi32(~0));
	__m128 screenMin = _mm_set1_ps(FLT_MAX);
	__m128 screenMax = _mm_set1_ps(-FLT_MAX);

	// Find the minimum of each component
	__m128 minvert = _mm_add_ps(cumulativeMatrix[3], _mm_add_ps(_mm_add_ps(_mm_min_ps(xRow[0], xRow[1]), _mm_min_ps(yRow[0], yRow[1])), _mm_min_ps(zRow[0], zRow[1])));
	float minW = minvert.m128_f32[3];
	if (minW < 0.00000001f)
		return ePT_VISIBLE;

	for (UINT i = 0; i < AABB_VERTICES; i++)
	{
		// Transform the vertex
		__m128 vert = cumulativeMatrix[3];
		vert += xRow[sBBxInd[i]];
		vert += yRow[sBByInd[i]];
		vert += zRow[sBBzInd[i]];

		// We have inverted z; z is in front of near plane iff z <= w.
		__m128 vertZ = _mm_shuffle_ps(vert, vert, 0xaa); // vert.zzzz
		__m128 vertW = _mm_shuffle_ps(vert, vert, 0xff); // vert.wwww

		// project
		xformedPos[i] = _mm_div_ps(vert, vertW);

		// update bounds
		screenMin = _mm_min_ps(screenMin, xformedPos[i]);
		screenMax = _mm_max_ps(screenMax, xformedPos[i]);
	}

	MaskedOcclusionCulling::CullingResult res = moc->TestRect(screenMin.m128_f32[0], screenMin.m128_f32[1], screenMax.m128_f32[0], screenMax.m128_f32[1], minW);


	if (res == MaskedOcclusionCulling::VISIBLE)
		return ePT_UNSURE;
	return ePT_INVISIBLE;
}

//----------------------------------------------------------------
// Transforms the AABB vertices to screen space once every frame
// Also performs a coarse depth pre-test
//----------------------------------------------------------------
PreTestResult TransformedAABBoxAVX::TransformAndPreTestAABBox(__m128 xformedPos[], const __m128 cumulativeMatrix[4], const float *pDepthSummary)
{
	// w ends up being garbage, but it doesn't matter - we ignore it anyway.
	__m128 vCenter = _mm_loadu_ps(&mBBCenter.x);
	__m128 vHalf   = _mm_loadu_ps(&mBBHalf.x);

	__m128 vMin    = _mm_sub_ps(vCenter, vHalf);
	__m128 vMax    = _mm_add_ps(vCenter, vHalf);

	// transforms
	__m128 xRow[2], yRow[2], zRow[2];
	xRow[0] = _mm_shuffle_ps(vMin, vMin, 0x00) * cumulativeMatrix[0];
	xRow[1] = _mm_shuffle_ps(vMax, vMax, 0x00) * cumulativeMatrix[0];
	yRow[0] = _mm_shuffle_ps(vMin, vMin, 0x55) * cumulativeMatrix[1];
	yRow[1] = _mm_shuffle_ps(vMax, vMax, 0x55) * cumulativeMatrix[1];
	zRow[0] = _mm_shuffle_ps(vMin, vMin, 0xaa) * cumulativeMatrix[2];
	zRow[1] = _mm_shuffle_ps(vMax, vMax, 0xaa) * cumulativeMatrix[2];

	__m128 zAllIn = _mm_castsi128_ps(_mm_set1_epi32(~0));
	__m128 screenMin = _mm_set1_ps(FLT_MAX);
	__m128 screenMax = _mm_set1_ps(-FLT_MAX);

	for(UINT i = 0; i < AABB_VERTICES; i++)
	{
		// Transform the vertex
		__m128 vert = cumulativeMatrix[3];
		vert += xRow[sBBxInd[i]];
		vert += yRow[sBByInd[i]];
		vert += zRow[sBBzInd[i]];

		// We have inverted z; z is in front of near plane iff z <= w.
		__m128 vertZ = _mm_shuffle_ps(vert, vert, 0xaa); // vert.zzzz
		__m128 vertW = _mm_shuffle_ps(vert, vert, 0xff); // vert.wwww
		__m128 zIn = _mm_cmple_ps(vertZ, vertW);
		zAllIn = _mm_and_ps(zAllIn, zIn);

		// project
		xformedPos[i] = _mm_div_ps(vert, vertW);
		
	    // update bounds
	    screenMin = _mm_min_ps(screenMin, xformedPos[i]);
	    screenMax = _mm_max_ps(screenMax, xformedPos[i]);
	}

	// if any of the verts are z-clipped, we (conservatively) say the box is in
	if(_mm_movemask_ps(zAllIn) != 0xf)
		return ePT_VISIBLE;

	// Clip against screen bounds
	screenMin = _mm_max_ps(screenMin, _mm_setr_ps(0.0f, 0.0f, 0.0f, -FLT_MAX));
	screenMax = _mm_min_ps(screenMax, _mm_setr_ps((float) (SCREENW - 1), (float) (SCREENH - 1), 1.0f, FLT_MAX));

	// Quick rejection test
	if(_mm_movemask_ps(_mm_cmplt_ps(screenMax, screenMin)))
		return ePT_INVISIBLE;

	// Prepare integer bounds
	__m128 minMaxXY = _mm_shuffle_ps(screenMin, screenMax, 0x44); // minX,minY,maxX,maxY
	__m128i minMaxXYi = _mm_cvtps_epi32(minMaxXY);
	__m128i minMaxXYis = _mm_srai_epi32(minMaxXYi, 3);

	__m128 maxZ = _mm_shuffle_ps(screenMax, screenMax, 0xaa);

	// Traverse all 8x8 blocks covered by 2d screen-space BBox;
	// if we know for sure that this box is behind the geometry we know is there,
	// we can stop.
	int rX0 = minMaxXYis.m128i_i32[0];
	int rY0 = minMaxXYis.m128i_i32[1];
	int rX1 = minMaxXYis.m128i_i32[2];
	int rY1 = minMaxXYis.m128i_i32[3];

	__m128 anyCloser = _mm_setzero_ps();
	for(int by = rY0; by <= rY1; by++)
	{
		const float *srcRow = pDepthSummary + by * (SCREENW/BLOCK_SIZE);

		// If for any 8x8 block, maxZ is not less than (=behind) summarized
		// min Z, box might be visible.
		for(int bx = rX0; bx <= rX1; bx++)
		{
			anyCloser = _mm_or_ps(anyCloser, _mm_cmpnlt_ss(maxZ, _mm_load_ss(&srcRow[bx])));
		}

		if(_mm_movemask_ps(anyCloser))
		{
			return ePT_UNSURE; // okay, box might be in
		}
	}

	// If we get here, we know for sure that the box is fully behind the stuff in the
	// depth buffer.
	return ePT_INVISIBLE;
}

void TransformedAABBoxAVX::Gather(vFloat4 pOut[3], UINT triId, const __m128 xformedPos[], UINT idx)
{
	for(int i = 0; i < 3; i++)
	{
		UINT ind0 = sBBIndexList[triId*3 + i + 0];
		UINT ind1 = sBBIndexList[triId*3 + i + 3];
		UINT ind2 = sBBIndexList[triId*3 + i + 6];
		UINT ind3 = sBBIndexList[triId*3 + i + 9];

		__m128 v0 = xformedPos[ind0];	// x0 y0 z0 w0
		__m128 v1 = xformedPos[ind1];	// x1 y1 z1 w1
		__m128 v2 = xformedPos[ind2];	// x2 y2 z2 w2
		__m128 v3 = xformedPos[ind3];	// x3 y3 z3 w3
		_MM_TRANSPOSE4_PS(v0, v1, v2, v3);
		pOut[i].X = v0;
		pOut[i].Y = v1;
		pOut[i].Z = v2;
		pOut[i].W = v3;
	}
}

//-----------------------------------------------------------------------------------------
// Rasterize the occludee AABB and depth test it against the CPU rasterized depth buffer
// If any of the rasterized AABB pixels passes the depth test exit early and mark the occludee
// as visible. If all rasterized AABB pixels are occluded then the occludee is culled
//-----------------------------------------------------------------------------------------
bool TransformedAABBoxAVX::RasterizeAndDepthTestAABBox(UINT *pRenderTargetPixels, const __m128 pXformedPos[], UINT idx)
{
	// Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
	// Denormal are zero (DAZ) is bit 6 and Flush to zero (FZ) is bit 15. 
	// so to enable the two to have to set bits 6 and 15 which 1000 0000 0100 0000 = 0x8040
	_mm_setcsr( _mm_getcsr() | 0x8040 );

	__m256i colOffset = _mm256_setr_epi32(0, 1, 2, 3, 0, 1, 2, 3);
	__m256i rowOffset = _mm256_setr_epi32(0, 0, 0, 0, 1, 1, 1, 1);
	float* pDepthBuffer = (float*)pRenderTargetPixels;
	
	// Rasterize the AABB triangles 4 at a time
	for(UINT i = 0; i < AABB_TRIANGLES; i += SSE)
	{
		vFloat4 xformedPos[3];
		Gather(xformedPos, i, pXformedPos, idx);

		// use fixed-point only for X and Y.  Avoid work for Z and W.
        __m128i fxPtX[3], fxPtY[3];
		for(int m = 0; m < 3; m++)
		{
			fxPtX[m] = _mm_cvtps_epi32(xformedPos[m].X);
			fxPtY[m] = _mm_cvtps_epi32(xformedPos[m].Y);
		}

		// Fab(x, y) =     Ax       +       By     +      C              = 0
		// Fab(x, y) = (ya - yb)x   +   (xb - xa)y + (xa * yb - xb * ya) = 0
		// Compute A = (ya - yb) for the 3 line segments that make up each triangle
		__m128i A0 = _mm_sub_epi32(fxPtY[1], fxPtY[2]);
		__m128i A1 = _mm_sub_epi32(fxPtY[2], fxPtY[0]);
		__m128i A2 = _mm_sub_epi32(fxPtY[0], fxPtY[1]);

		// Compute B = (xb - xa) for the 3 line segments that make up each triangle
		__m128i B0 = _mm_sub_epi32(fxPtX[2], fxPtX[1]);
		__m128i B1 = _mm_sub_epi32(fxPtX[0], fxPtX[2]);
		__m128i B2 = _mm_sub_epi32(fxPtX[1], fxPtX[0]);

		// Compute C = (xa * yb - xb * ya) for the 3 line segments that make up each triangle
		__m128i C0 = _mm_sub_epi32(_mm_mullo_epi32(fxPtX[1], fxPtY[2]), _mm_mullo_epi32(fxPtX[2], fxPtY[1]));
		__m128i C1 = _mm_sub_epi32(_mm_mullo_epi32(fxPtX[2], fxPtY[0]), _mm_mullo_epi32(fxPtX[0], fxPtY[2]));
		__m128i C2 = _mm_sub_epi32(_mm_mullo_epi32(fxPtX[0], fxPtY[1]), _mm_mullo_epi32(fxPtX[1], fxPtY[0]));

		// Compute triangle area
		__m128i triArea = _mm_mullo_epi32(B2, A1);
		triArea = _mm_sub_epi32(triArea, _mm_mullo_epi32(B1, A2));
		__m128 oneOverTriArea = _mm_rcp_ps(_mm_cvtepi32_ps(triArea));

		__m128 Z[3];
		Z[0] = xformedPos[0].Z;
		Z[1] = _mm_mul_ps(_mm_sub_ps(xformedPos[1].Z, Z[0]), oneOverTriArea);
		Z[2] = _mm_mul_ps(_mm_sub_ps(xformedPos[2].Z, Z[0]), oneOverTriArea);
		
		// Use bounding box traversal strategy to determine which pixels to rasterize 
		//__m128i startX = _mm_and_si128(HelperSSE::Max(HelperSSE::Min(HelperSSE::Min(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(0)), _mm_set1_epi32(~1));
		__m128i startX = _mm_and_si128(HelperSSE::Max(HelperSSE::Min(HelperSSE::Min(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(0)), _mm_set1_epi32(~3));
		__m128i endX = HelperSSE::Min(HelperSSE::Max(HelperSSE::Max(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(SCREENW - 1));

		__m128i startY = _mm_and_si128(HelperSSE::Max(HelperSSE::Min(HelperSSE::Min(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(0)), _mm_set1_epi32(~1));
		__m128i endY = HelperSSE::Min(HelperSSE::Max(HelperSSE::Max(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(SCREENH - 1));

		// Now we have 4 triangles set up.  Rasterize them each individually.
        for(int lane=0; lane < SSE; lane++)
        {
			// Skip triangle if area is zero 
			if(triArea.m128i_i32[lane] <= 0)
			{
				continue;
			}

			// Extract this triangle's properties from the SIMD versions
			__m256 zz[3];
			for (int vv = 0; vv < 3; vv++)
			{
				zz[vv] = _mm256_set1_ps(Z[vv].m128_f32[lane]);
			}

			int startXx = startX.m128i_i32[lane];
			int endXx = endX.m128i_i32[lane];
			int startYy = startY.m128i_i32[lane];
			int endYy = endY.m128i_i32[lane];

			__m256i aa0 = _mm256_set1_epi32(A0.m128i_i32[lane]);
			__m256i aa1 = _mm256_set1_epi32(A1.m128i_i32[lane]);
			__m256i aa2 = _mm256_set1_epi32(A2.m128i_i32[lane]);

			__m256i bb0 = _mm256_set1_epi32(B0.m128i_i32[lane]);
			__m256i bb1 = _mm256_set1_epi32(B1.m128i_i32[lane]);
			__m256i bb2 = _mm256_set1_epi32(B2.m128i_i32[lane]);

			__m256i aa0Inc = _mm256_slli_epi32(aa0, 2);
			__m256i aa1Inc = _mm256_slli_epi32(aa1, 2);
			__m256i aa2Inc = _mm256_slli_epi32(aa2, 2);

			__m256i bb0Inc = _mm256_slli_epi32(bb0, 1);
			__m256i bb1Inc = _mm256_slli_epi32(bb1, 1);
			__m256i bb2Inc = _mm256_slli_epi32(bb2, 1);

			__m256i row, col;

			// Traverse pixels in 2x4 blocks and store 2x4 pixel quad depths contiguously in memory ==> 2*X
			// This method provides better performance
			int	rowIdx = (startYy * SCREENW + 2 * startXx);

			col = _mm256_add_epi32(colOffset, _mm256_set1_epi32(startXx));
			__m256i aa0Col = _mm256_mullo_epi32(aa0, col);
			__m256i aa1Col = _mm256_mullo_epi32(aa1, col);
			__m256i aa2Col = _mm256_mullo_epi32(aa2, col);

			row = _mm256_add_epi32(rowOffset, _mm256_set1_epi32(startYy));
			__m256i bb0Row = _mm256_add_epi32(_mm256_mullo_epi32(bb0, row), _mm256_set1_epi32(C0.m128i_i32[lane]));
			__m256i bb1Row = _mm256_add_epi32(_mm256_mullo_epi32(bb1, row), _mm256_set1_epi32(C1.m128i_i32[lane]));
			__m256i bb2Row = _mm256_add_epi32(_mm256_mullo_epi32(bb2, row), _mm256_set1_epi32(C2.m128i_i32[lane]));

			__m256i sum0Row = _mm256_add_epi32(aa0Col, bb0Row);
			__m256i sum1Row = _mm256_add_epi32(aa1Col, bb1Row);
			__m256i sum2Row = _mm256_add_epi32(aa2Col, bb2Row);

			__m256 zx = _mm256_mul_ps(_mm256_cvtepi32_ps(aa1Inc), zz[1]);
			zx = _mm256_add_ps(zx, _mm256_mul_ps(_mm256_cvtepi32_ps(aa2Inc), zz[2]));

			// Incrementally compute Fab(x, y) for all the pixels inside the bounding box formed by (startX, endX) and (startY, endY)
			for (int r = startYy; r < endYy; r += 2,
				rowIdx += 2 * SCREENW,
				sum0Row = _mm256_add_epi32(sum0Row, bb0Inc),
				sum1Row = _mm256_add_epi32(sum1Row, bb1Inc),
				sum2Row = _mm256_add_epi32(sum2Row, bb2Inc))
			{
				// Compute barycentric coordinates 
				int index = rowIdx;
				__m256i alpha = sum0Row;
				__m256i beta = sum1Row;
				__m256i gama = sum2Row;

				//Compute barycentric-interpolated depth
				__m256 depth = zz[0];
				depth = _mm256_add_ps(depth, _mm256_mul_ps(_mm256_cvtepi32_ps(beta), zz[1]));
				depth = _mm256_add_ps(depth, _mm256_mul_ps(_mm256_cvtepi32_ps(gama), zz[2]));
				__m256i anyOut = _mm256_setzero_si256();

				for (int c = startXx; c < endXx; c += 4,
					index += 8,
					alpha = _mm256_add_epi32(alpha, aa0Inc),
					beta = _mm256_add_epi32(beta, aa1Inc),
					gama = _mm256_add_epi32(gama, aa2Inc),
					depth = _mm256_add_ps(depth, zx))
				{
					//Test Pixel inside triangle
					__m256i mask = _mm256_or_si256(_mm256_or_si256(alpha, beta), gama);

					__m256 previousDepthValue = _mm256_loadu_ps(&pDepthBuffer[index]);
					__m256 depthMask = _mm256_cmp_ps(depth, previousDepthValue, 0x1D);
					__m256i finalMask = _mm256_andnot_si256(mask, _mm256_castps_si256(depthMask));
					anyOut = _mm256_or_si256(anyOut, finalMask);
				}//for each column	

				if (!_mm256_testz_si256(anyOut, _mm256_set1_epi32(0x80000000)))
				{
					return true; //early exit
				}
			}// for each row
		}// for each triangle
	}// for each set of SIMD# triangles

	return false;
}

//-----------------------------------------------------------------------------------------
// Rasterize the occludee AABB and depth test it against the CPU rasterized depth buffer
// If any of the rasterized AABB pixels passes the depth test exit early and mark the occludee
// as visible. If all rasterized AABB pixels are occluded then the occludee is culled
//-----------------------------------------------------------------------------------------
bool TransformedAABBoxAVX::RasterizeAndDepthTestAABBox(MaskedOcclusionCulling *moc, const __m128 pXformedPos[])
{
	MaskedOcclusionCulling::CullingResult res;
	res = moc->TestTriangles((float*)pXformedPos, sBBIndexList, 12, nullptr, MaskedOcclusionCulling::BACKFACE_CW, MaskedOcclusionCulling::CLIP_PLANE_ALL, MaskedOcclusionCulling::VertexLayout(sizeof(__m128), 4, 12));
	if (res != MaskedOcclusionCulling::VISIBLE)
		return false;
	else
		return true;
}
