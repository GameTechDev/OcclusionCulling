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

#include "TransformedAABBoxScalar.h"

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
void TransformedAABBoxScalar::CreateAABBVertexIndexList(CPUTModelDX11 *pModel)
{
	mWorldMatrix = *pModel->GetWorldMatrix();
	pModel->GetBoundsObjectSpace(&mBBCenter, &mBBHalf);
	mRadiusSq = mBBHalf.lengthSq();
	pModel->GetBoundsWorldSpace(&mBBCenterWS, &mBBHalfWS);	
}

//----------------------------------------------------------------
// Determine is model is inside view frustum
//----------------------------------------------------------------
bool TransformedAABBoxScalar::IsInsideViewFrustum(CPUTCamera *pCamera)
{
	return pCamera->mFrustum.IsVisible(mBBCenterWS, mBBHalfWS);
}

//----------------------------------------------------------------------------
// Determine if the occludee size is too small and if so avoid drawing it
//----------------------------------------------------------------------------
bool TransformedAABBoxScalar::IsTooSmall(const BoxTestSetupScalar &setup, float4x4 &cumulativeMatrix)
{
	cumulativeMatrix = mWorldMatrix * setup.mViewProjViewport;

	float w = mBBCenter.x * cumulativeMatrix.r0.w + 
			  mBBCenter.y * cumulativeMatrix.r1.w + 
			  mBBCenter.z * cumulativeMatrix.r2.w + 
			  cumulativeMatrix.r3.w;

	if( w > 1.0f )
	{
		return mRadiusSq < w * setup.radiusThreshold;
	}
	return false;
}

//----------------------------------------------------------------
// Trasforms the AABB vertices to screen space once every frame
//----------------------------------------------------------------
bool TransformedAABBoxScalar::TransformAABBox(float4 xformedPos[], const float4x4 &cumulativeMatrix)
{
	float4 vCenter = float4(mBBCenter.x, mBBCenter.y, mBBCenter.z, 1.0);
	float4 vHalf   = float4(mBBHalf.x, mBBHalf.y, mBBHalf.z, 1.0);

	float4 vMin    = vCenter - vHalf;
	float4 vMax    = vCenter + vHalf;

	// transforms
	float4 xRow[2], yRow[2], zRow[2];
	xRow[0] = float4(vMin.x, vMin.x, vMin.x, vMin.x) * cumulativeMatrix.r0;
	xRow[1] = float4(vMax.x, vMax.x, vMax.x, vMax.x) * cumulativeMatrix.r0;
	yRow[0] = float4(vMin.y, vMin.y, vMin.y, vMin.y) * cumulativeMatrix.r1;
	yRow[1] = float4(vMax.y, vMax.y, vMax.y, vMax.y) * cumulativeMatrix.r1;
	zRow[0] = float4(vMin.z, vMin.z, vMin.z, vMin.z) * cumulativeMatrix.r2;
	zRow[1] = float4(vMax.z, vMax.z, vMax.z, vMax.z) * cumulativeMatrix.r2;
	
	bool zAllIn = true;
	
	for(UINT i = 0; i < AABB_VERTICES; i++)
	{
		float4 vert = cumulativeMatrix.r3;
		vert += xRow[sBBxInd[i]];
		vert += yRow[sBByInd[i]];
		vert += zRow[sBBzInd[i]];

		zAllIn =  vert.z <= vert.w ?  zAllIn & true : zAllIn & false;

		xformedPos[i] = vert/ vert.w;
	}
	return zAllIn;
}

void TransformedAABBoxScalar::Gather(float4 pOut[3], UINT triId, const float4 xformedPos[])
{
	for(UINT i = 0; i < 3; i++)
	{
		UINT index = sBBIndexList[(triId * 3) + i];
		pOut[i] = xformedPos[index];	
	}
}

//-----------------------------------------------------------------------------------------
// Rasterize the occludee AABB and depth test it against the CPU rasterized depth buffer
// If any of the rasterized AABB pixels passes the depth test exit early and mark the occludee
// as visible. If all rasterized AABB pixels are occluded then the occludee is culled
//-----------------------------------------------------------------------------------------
bool TransformedAABBoxScalar::RasterizeAndDepthTestAABBox(UINT *pRenderTargetPixels, const float4 pXformedPos[], UINT idx)
{
	float* pDepthBuffer = (float*)pRenderTargetPixels; 
	
	// Rasterize the AABB triangles
	for(UINT i = 0; i < AABB_TRIANGLES; i++)
	{
		float4 xformedPos[3];
		Gather(xformedPos, i, pXformedPos);

		int fxPtX[3], fxPtY[3];
		float Z[3];
		for(UINT j = 0; j < 3; j++)
		{
			fxPtX[j] = (int)(xformedPos[j].x + 0.5);
			fxPtY[j] = (int)(xformedPos[j].y + 0.5);
			Z[j] = xformedPos[j].z;
		}

		// Fab(x, y) =     Ax       +       By     +      C              = 0
		// Fab(x, y) = (ya - yb)x   +   (xb - xa)y + (xa * yb - xb * ya) = 0
		// Compute A = (ya - yb) for the 3 line segments that make up each triangle
		int A0 = fxPtY[1] - fxPtY[2];
		int A1 = fxPtY[2] - fxPtY[0];
		int A2 = fxPtY[0] - fxPtY[1];

		// Compute B = (xb - xa) for the 3 line segments that make up each triangle
		int B0 = fxPtX[2] - fxPtX[1];
		int B1 = fxPtX[0] - fxPtX[2];
		int B2 = fxPtX[1] - fxPtX[0];

		// Compute C = (xa * yb - xb * ya) for the 3 line segments that make up each triangle
		int C0 = fxPtX[1] * fxPtY[2] - fxPtX[2] * fxPtY[1];
		int C1 = fxPtX[2] * fxPtY[0] - fxPtX[0] * fxPtY[2];
		int C2 = fxPtX[0] * fxPtY[1] - fxPtX[1] * fxPtY[0];

		// Compute triangle area
		int triArea = (fxPtX[1] - fxPtX[0]) * (fxPtY[2] - fxPtY[0]) - (fxPtX[0] - fxPtX[2]) * (fxPtY[0] - fxPtY[1]);
		float oneOverTriArea = (1.0f/float(triArea));

		Z[1] = (Z[1] - Z[0]) * oneOverTriArea;
		Z[2] = (Z[2] - Z[0]) * oneOverTriArea;

		// Use bounding box traversal strategy to determine which pixels to rasterize 
		int startX = max(min(min(fxPtX[0], fxPtX[1]), fxPtX[2]), 0) & int(0xFFFFFFFE);
		int endX   = min(max(max(fxPtX[0], fxPtX[1]), fxPtX[2]), SCREENW-1);

		int startY = max(min(min(fxPtY[0], fxPtY[1]), fxPtY[2]), 0) & int(0xFFFFFFFE);
		int endY   = min(max(max(fxPtY[0], fxPtY[1]), fxPtY[2]), SCREENH-1);

		//Skip triangle if area is zero 
		if(triArea <= 0)
		{
			continue;
		}

		int rowIdx = (startY * SCREENW + startX);
		int col = startX;
		int row = startY;
		
		int alpha0 = (A0 * col) + (B0 * row) + C0;
		int beta0 = (A1 * col) + (B1 * row) + C1;
		int gama0 = (A2 * col) + (B2 * row) + C2;
		
		float zx = A1 * Z[1] + A2 * Z[2];

		// Incrementally compute Fab(x, y) for all the pixels inside the bounding box formed by (startX, endX) and (startY, endY)
		for(int r = startY; r < endY; r++,
									  row++,
									  rowIdx = rowIdx + SCREENW,
									  alpha0 += B0,
									  beta0 += B1,
									  gama0 += B2)									 
		{
			// Compute barycentric coordinates 
			int index = rowIdx;
			int alpha = alpha0;
			int beta = beta0;
			int gama = gama0;
			
			float depth = Z[0] + Z[1] * beta + Z[2] * gama;
			bool anyOut = false;

			for(int c = startX; c < endX; c++,
   										  index++,
										  alpha = alpha + A0,
										  beta  = beta  + A1,
										  gama  = gama  + A2,
										  depth += zx)
			{
				//Test Pixel inside triangle
				int mask = alpha | beta | gama;
				float previousDepthValue = pDepthBuffer[index];
				anyOut = (mask > 0 && depth >= previousDepthValue) ? anyOut | true : anyOut | false;
			}//for each column	
			
			if(anyOut)
			{
				return true;
			}														
		}// for each row
	}// for each triangle
	return false;
}
