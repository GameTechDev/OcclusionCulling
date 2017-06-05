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
#include "DepthBufferRasterizerAVXST.h"

DepthBufferRasterizerAVXST::DepthBufferRasterizerAVXST()
	: DepthBufferRasterizerAVX()
{
	int size = SCREENH_IN_TILES * SCREENW_IN_TILES;
	
	mpBin[0] = new BinTriangle[size * MAX_TRIS_IN_BIN_ST];
	mpNumTrisInBin[0] = new USHORT[size];

	mpBin[1] = new BinTriangle[size * MAX_TRIS_IN_BIN_ST];
	mpNumTrisInBin[1] = new USHORT[size];
}

DepthBufferRasterizerAVXST::~DepthBufferRasterizerAVXST()
{
	SAFE_DELETE_ARRAY(mpBin[0]);
	SAFE_DELETE_ARRAY(mpNumTrisInBin[0]);

	SAFE_DELETE_ARRAY(mpBin[1]);
	SAFE_DELETE_ARRAY(mpNumTrisInBin[1]);
}

//------------------------------------------------------------------------------
// * Determine if the occludee model is inside view frustum
// * Transform the occluder models on the CPU
// * Bin the occluder triangles into tiles that the frame buffer is divided into
// * Rasterize the occluder triangles to the CPU depth buffer
//-------------------------------------------------------------------------------
void DepthBufferRasterizerAVXST::TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx)
{
	QueryPerformanceCounter(&mStartTime[idx]);
	mpCamera[idx] = pCamera;
	
	BoxTestSetupSSE setup;
	setup.Init(mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrix, pCamera, mOccluderSizeThreshold);

	if(mEnableFCulling)
	{
		for(UINT i = 0; i < mNumModels1; i++)
		{
			mpTransformedModels1[i].InsideViewFrustum(setup, idx);
		}
	}
	else
	{
		for(UINT i = 0; i < mNumModels1; i++)
		{
			mpTransformedModels1[i].TooSmall(setup, idx);
		}
	}

	ActiveModels(idx);
	TransformMeshes(idx);
	BinTransformedMeshes(idx);
	for(UINT i = 0; i < NUM_TILES; i++)
	{
		RasterizeBinnedTrianglesToDepthBuffer(i, idx);
	}

	QueryPerformanceCounter(&mStopTime[idx][0]);
	mRasterizeTime[mTimeCounter++] = ((double)(mStopTime[idx][0].QuadPart - mStartTime[idx].QuadPart)) / ((double)glFrequency.QuadPart);
	mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;
}

void DepthBufferRasterizerAVXST::ActiveModels(UINT idx)
{
	ResetActive(idx);
	for (UINT i = 0; i < mNumModels1; i++)
	{
		if(mpTransformedModels1[i].IsRasterized2DB(idx))
		{
			Activate(i, idx);
		}
	}
}

//-------------------------------------------------------------------
// Trasforms the occluder vertices to screen space once every frame
//-------------------------------------------------------------------
void DepthBufferRasterizerAVXST::TransformMeshes(UINT idx)
{
	for(UINT active = 0; active < mNumModelsA[idx]; active++)
    {
		UINT ss = mpModelIndexA[idx][active];
		UINT thisSurfaceVertexCount = mpTransformedModels1[ss].GetNumVertices();
        
        mpTransformedModels1[ss].TransformMeshes(0, thisSurfaceVertexCount - 1, mpCamera[idx], idx);
    }
}

//-------------------------------------------------
// Bins the transformed triangles into tiles
//-------------------------------------------------
void DepthBufferRasterizerAVXST::BinTransformedMeshes(UINT idx)
{
	// Reset the bin count.  Note the data layout makes this traversal a bit awkward.
    // We can't just use memset() because the last array index isn't what's varying.
    // However, this should make the real use of this structure go faster.
	for(UINT yy = 0; yy < SCREENH_IN_TILES; yy++)
    {
		UINT offset = YOFFSET1_ST * yy;
        for(UINT xx = 0; xx < SCREENW_IN_TILES; xx++)
        {
			UINT index = offset + (XOFFSET1_ST * xx);
            mpNumTrisInBin[idx][index] = 0;
	    }
    }

	// Now, process all of the surfaces that contain this task's triangle range.
	for(UINT active = 0; active < mNumModelsA[idx]; active++)
    {
		UINT ss = mpModelIndexA[idx][active];
		UINT thisSurfaceTriangleCount = mpTransformedModels1[ss].GetNumTriangles();
        
		mpTransformedModels1[ss].BinTransformedTrianglesST(0, ss, 0, thisSurfaceTriangleCount - 1, mpBin[idx], mpNumTrisInBin[idx], idx);
	}
}

//-------------------------------------------------------------------------------
// For each tile go through all the bins and process all the triangles in it.
// Rasterize each triangle to the CPU depth buffer. 
//-------------------------------------------------------------------------------
void DepthBufferRasterizerAVXST::RasterizeBinnedTrianglesToDepthBuffer(UINT tileId, UINT idx)
{
	// Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
	_mm_setcsr( _mm_getcsr() | 0x8040 );

	__m256i colOffset = _mm256_setr_epi32(0, 1, 2, 3, 0, 1, 2, 3);
	__m256i rowOffset = _mm256_setr_epi32(0, 0, 0, 0, 1, 1, 1, 1);
	
	float* pDepthBuffer = (float*)mpRenderTargetPixels[idx];

	// Based on TaskId determine which tile to process
	UINT screenWidthInTiles = SCREENW/TILE_WIDTH_IN_PIXELS;
    UINT tileX = tileId % screenWidthInTiles;
    UINT tileY = tileId / screenWidthInTiles;

    int tileStartX = tileX * TILE_WIDTH_IN_PIXELS;
	int tileEndX   = min(tileStartX + TILE_WIDTH_IN_PIXELS - 1, SCREENW - 1);
	
	int tileStartY = tileY * TILE_HEIGHT_IN_PIXELS;
	int tileEndY   = min(tileStartY + TILE_HEIGHT_IN_PIXELS - 1, SCREENH - 1);

	ClearDepthTile(tileStartX, tileStartY, tileEndX+1, tileEndY+1, idx);

	UINT offset1 = YOFFSET1_ST * tileY + XOFFSET1_ST * tileX;
	UINT offset2 = YOFFSET2_ST * tileY + XOFFSET2_ST * tileX;

	UINT bin = 0;
	UINT binIndex = 0;
	UINT numTrisInBin = mpNumTrisInBin[idx][offset1 + bin];

	__declspec(align(32)) int vIndex[8] = { 0, 24, 48, 72, 96, 120, 144, 168 };
	__m256i gatherBuf[6];
	__m256i gatherBufvindex = _mm256_load_si256((const __m256i *)vIndex);
	__m256i four = _mm256_set1_epi32(4);
	bool done = false;
	bool allBinsEmpty = true;
	mNumRasterizedTris[idx][tileId] = numTrisInBin;
	
	while (!done)
	{
		// Loop through all the bins and process 8 binned triangles at a time
		int numSimdTris = 0;
		int numTrisToProcess = 0;
		{
			while (numTrisInBin <= 0)
			{
				// This bin is empty.  Move to next bin.
				if (++bin >= 1)
				{
					break;
				}
				numTrisInBin = mpNumTrisInBin[idx][offset1 + bin];
				mNumRasterizedTris[idx][tileId] += numTrisInBin;
				binIndex = 0;
			}

			if (numTrisInBin > 0)
			{
				if (numTrisInBin >= 8)
				{
					gatherBufvindex = _mm256_load_si256((const __m256i *)vIndex);
					numTrisToProcess = 8;
				}
				else
				{
					gatherBufvindex = _mm256_set1_epi32(0);
					for (UINT i = 0; i < numTrisInBin; i++)
					{
						gatherBufvindex.m256i_i32[i] = i * 24;
					}
					numTrisToProcess = numTrisInBin;
				}

				const BinTriangle *pTri = &mpBin[idx][offset2 + bin * MAX_TRIS_IN_BIN_MT + binIndex];
				gatherBuf[0] = _mm256_i32gather_epi32((const int *)&pTri->vert[0].xy, gatherBufvindex, 1);
				gatherBufvindex = _mm256_add_epi32(gatherBufvindex, four);
				gatherBuf[1] = _mm256_i32gather_epi32((const int *)&pTri->vert[0].xy, gatherBufvindex, 1);
				gatherBufvindex = _mm256_add_epi32(gatherBufvindex, four);
				gatherBuf[2] = _mm256_i32gather_epi32((const int *)&pTri->vert[0].xy, gatherBufvindex, 1);
				gatherBufvindex = _mm256_add_epi32(gatherBufvindex, four);
				gatherBuf[3] = _mm256_i32gather_epi32((const int *)&pTri->vert[0].xy, gatherBufvindex, 1);
				gatherBufvindex = _mm256_add_epi32(gatherBufvindex, four);
				gatherBuf[4] = _mm256_i32gather_epi32((const int *)&pTri->vert[0].xy, gatherBufvindex, 1);
				gatherBufvindex = _mm256_add_epi32(gatherBufvindex, four);
				gatherBuf[5] = _mm256_i32gather_epi32((const int *)&pTri->vert[0].xy, gatherBufvindex, 1);

				numSimdTris += numTrisToProcess;
				binIndex += numTrisToProcess;
				numTrisInBin -= numTrisToProcess;
				allBinsEmpty = false;
			}
		}

		done = bin >= NUM_XFORMVERTS_TASKS;
		if (allBinsEmpty)
		{
			return;
		}

		__m256i fxPtX[3], fxPtY[3];
		{
			fxPtX[0] = _mm256_srai_epi32(_mm256_slli_epi32(gatherBuf[0], 16), 16);
			fxPtY[0] = _mm256_srai_epi32(gatherBuf[0], 16);
			fxPtX[1] = _mm256_srai_epi32(_mm256_slli_epi32(gatherBuf[1], 16), 16);
			fxPtY[1] = _mm256_srai_epi32(gatherBuf[1], 16);
			fxPtX[2] = _mm256_srai_epi32(_mm256_slli_epi32(gatherBuf[2], 16), 16);
			fxPtY[2] = _mm256_srai_epi32(gatherBuf[2], 16);
		}

		// Fab(x, y) =     Ax       +       By     +      C              = 0
		// Fab(x, y) = (ya - yb)x   +   (xb - xa)y + (xa * yb - xb * ya) = 0
		// Compute A = (ya - yb) for the 3 line segments that make up each triangle
		__m256i AA0 = _mm256_sub_epi32(fxPtY[1], fxPtY[2]);
		__m256i AA1 = _mm256_sub_epi32(fxPtY[2], fxPtY[0]);
		__m256i AA2 = _mm256_sub_epi32(fxPtY[0], fxPtY[1]);

		// Compute B = (xb - xa) for the 3 line segments that make up each triangle
		__m256i BB0 = _mm256_sub_epi32(fxPtX[2], fxPtX[1]);
		__m256i BB1 = _mm256_sub_epi32(fxPtX[0], fxPtX[2]);
		__m256i BB2 = _mm256_sub_epi32(fxPtX[1], fxPtX[0]);

		// Compute C = (xa * yb - xb * ya) for the 3 line segments that make up each triangle
		__m256i CC0 = _mm256_sub_epi32(_mm256_mullo_epi32(fxPtX[1], fxPtY[2]), _mm256_mullo_epi32(fxPtX[2], fxPtY[1]));
		__m256i CC1 = _mm256_sub_epi32(_mm256_mullo_epi32(fxPtX[2], fxPtY[0]), _mm256_mullo_epi32(fxPtX[0], fxPtY[2]));
		__m256i CC2 = _mm256_sub_epi32(_mm256_mullo_epi32(fxPtX[0], fxPtY[1]), _mm256_mullo_epi32(fxPtX[1], fxPtY[0]));

		__m256i startXAVX = _mm256_and_si256(Max(Min(Min(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm256_set1_epi32(tileStartX)), _mm256_set1_epi32(0xFFFFFFFC));
		__m256i endXAVX = Min(_mm256_add_epi32(Max(Max(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm256_set1_epi32(1)), _mm256_set1_epi32(tileEndX));

		__m256i startYAVX = _mm256_and_si256(Max(Min(Min(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm256_set1_epi32(tileStartY)), _mm256_set1_epi32(0xFFFFFFFE));
		__m256i endYAVX = Min(_mm256_add_epi32(Max(Max(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm256_set1_epi32(1)), _mm256_set1_epi32(tileEndY));

		// Now we have 8 triangles set up.  Rasterize them each individually.
		for (int lane = 0; lane < numSimdTris; lane++)
		{
			// Extract this triangle's properties from the SIMD versions
			__m256 zz[3];
			zz[0] = _mm256_castsi256_ps(_mm256_set1_epi32(gatherBuf[3].m256i_i32[lane]));
			zz[1] = _mm256_castsi256_ps(_mm256_set1_epi32(gatherBuf[4].m256i_i32[lane]));
			zz[2] = _mm256_castsi256_ps(_mm256_set1_epi32(gatherBuf[5].m256i_i32[lane]));

			int startXx = startXAVX.m256i_i32[lane];
			int endXx = endXAVX.m256i_i32[lane];
			int startYy = startYAVX.m256i_i32[lane];
			int endYy = endYAVX.m256i_i32[lane];

			__m256i aa0 = _mm256_set1_epi32(AA0.m256i_i32[lane]);
			__m256i aa1 = _mm256_set1_epi32(AA1.m256i_i32[lane]);
			__m256i aa2 = _mm256_set1_epi32(AA2.m256i_i32[lane]);

			__m256i bb0 = _mm256_set1_epi32(BB0.m256i_i32[lane]);
			__m256i bb1 = _mm256_set1_epi32(BB1.m256i_i32[lane]);
			__m256i bb2 = _mm256_set1_epi32(BB2.m256i_i32[lane]);

			__m256i aa0Inc = _mm256_slli_epi32(aa0, 2);
			__m256i aa1Inc = _mm256_slli_epi32(aa1, 2);
			__m256i aa2Inc = _mm256_slli_epi32(aa2, 2);

			__m256i row, col;

			// Tranverse pixels in 2x4 blocks and store 2x4 pixel quad depths contiguously in memory ==> 2*X
			// This method provides better performance
			int rowIdx = (startYy * SCREENW + 2 * startXx);

			col = _mm256_add_epi32(colOffset, _mm256_set1_epi32(startXx));
			__m256i aa0Col = _mm256_mullo_epi32(aa0, col);
			__m256i aa1Col = _mm256_mullo_epi32(aa1, col);
			__m256i aa2Col = _mm256_mullo_epi32(aa2, col);

			row = _mm256_add_epi32(rowOffset, _mm256_set1_epi32(startYy));
			__m256i bb0Row = _mm256_add_epi32(_mm256_mullo_epi32(bb0, row), _mm256_set1_epi32(CC0.m256i_i32[lane]));
			__m256i bb1Row = _mm256_add_epi32(_mm256_mullo_epi32(bb1, row), _mm256_set1_epi32(CC1.m256i_i32[lane]));
			__m256i bb2Row = _mm256_add_epi32(_mm256_mullo_epi32(bb2, row), _mm256_set1_epi32(CC2.m256i_i32[lane]));

			__m256i sum0Row = _mm256_add_epi32(aa0Col, bb0Row);
			__m256i sum1Row = _mm256_add_epi32(aa1Col, bb1Row);
			__m256i sum2Row = _mm256_add_epi32(aa2Col, bb2Row);

			__m256i bb0Inc = _mm256_slli_epi32(bb0, 1);
			__m256i bb1Inc = _mm256_slli_epi32(bb1, 1);
			__m256i bb2Inc = _mm256_slli_epi32(bb2, 1);

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
					__m256 mergedDepth = _mm256_max_ps(depth, previousDepthValue);
					__m256 finalDepth = _mm256_blendv_ps(mergedDepth, previousDepthValue, _mm256_castsi256_ps(mask));
					_mm256_storeu_ps(&pDepthBuffer[index], finalDepth);
				}//for each column											
			}// for each row
		}// for each triangle
	}// for each set of SIMD# triangles

	// Summarize depth buffer
	CreateCoarseDepth(tileStartX, tileStartY, tileEndX+1, tileEndY+1, idx);
}


void DepthBufferRasterizerAVXST::ComputeR2DBTime(UINT idx)
{
}