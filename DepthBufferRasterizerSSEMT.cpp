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

#include "DepthBufferRasterizerSSEMT.h"
#include"HelperMT.h"

DepthBufferRasterizerSSEMT::DepthBufferRasterizerSSEMT()
	: DepthBufferRasterizerSSE()
{
	int size = SCREENH_IN_TILES * SCREENW_IN_TILES *  NUM_XFORMVERTS_TASKS;
	
	mpBin[0] = new BinTriangle[size * MAX_TRIS_IN_BIN_MT];
	mpNumTrisInBin[0] = (USHORT*)_aligned_malloc(size * sizeof(USHORT), 64);

	mpBin[1] = new BinTriangle[size * MAX_TRIS_IN_BIN_MT];
	mpNumTrisInBin[1] = (USHORT*)_aligned_malloc(size * sizeof(USHORT), 64);
}

DepthBufferRasterizerSSEMT::~DepthBufferRasterizerSSEMT()
{
	SAFE_DELETE_ARRAY(mpBin[0]);
	_aligned_free(mpNumTrisInBin[0]);

	SAFE_DELETE_ARRAY(mpBin[1]);
	_aligned_free(mpNumTrisInBin[1]);
}

void DepthBufferRasterizerSSEMT::InsideViewFrustum(VOID *taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->InsideViewFrustum(taskId, taskCount, pTaskData->idx);
}

//------------------------------------------------------------
// * Determine if the occluder model is inside view frustum
//------------------------------------------------------------
void DepthBufferRasterizerSSEMT::InsideViewFrustum(UINT taskId, UINT taskCount, UINT idx)
{
	UINT start, end;
	GetWorkExtent(&start, &end, taskId, taskCount, mNumModels1);

 	BoxTestSetupSSE setup;
    setup.Init(mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrix, mpCamera[idx], mOccluderSizeThreshold); 

	for(UINT i = start; i < end; i++)
	{
		mpTransformedModels1[i].InsideViewFrustum(setup,  idx);
	}
}

void DepthBufferRasterizerSSEMT::TooSmall(VOID *taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->TooSmall(taskId, taskCount, pTaskData->idx);
}

//------------------------------------------------------------
// * Determine if the occluder model is too small in screen space
//------------------------------------------------------------
void DepthBufferRasterizerSSEMT::TooSmall(UINT taskId, UINT taskCount, UINT idx)
{
	UINT start, end;
	GetWorkExtent(&start, &end, taskId, taskCount, mNumModels1);

 	BoxTestSetupSSE setup;
    setup.Init(mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrix, mpCamera[idx], mOccluderSizeThreshold); 

	for(UINT i = start; i < end; i++)
	{
		mpTransformedModels1[i].TooSmall(setup,  idx);
	}
}

void DepthBufferRasterizerSSEMT::ActiveModels(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->ActiveModels(taskId, pTaskData->idx);
}

void DepthBufferRasterizerSSEMT::ActiveModels(UINT taskId, UINT idx)
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

//------------------------------------------------------------------------------
// Create tasks to determine if the occluder model is within the viewing frustum 
// Create NUM_XFORMVERTS_TASKS to:
// * Transform the occluder models on the CPU
// * Bin the occluder triangles into tiles that the frame buffer is divided into
// * Rasterize the occluder triangles to the CPU depth buffer
//-------------------------------------------------------------------------------
void DepthBufferRasterizerSSEMT::TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx)
{
	static const unsigned int kNumOccluderVisTasks = 32;

	mTaskData[idx].idx = idx;
	mTaskData[idx].pDBR = this;

	QueryPerformanceCounter(&mStartTime[idx]);
	mpCamera[idx] = pCamera;
	
	if(mEnableFCulling)
	{
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::InsideViewFrustum, &mTaskData[idx], kNumOccluderVisTasks, NULL, 0, "Is Visible", &gInsideViewFrustum[idx]);
		
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::ActiveModels, &mTaskData[idx], 1, &gInsideViewFrustum[idx], 1, "IsActive", &gActiveModels[idx]);
		
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::TransformMeshes, &mTaskData[idx], NUM_XFORMVERTS_TASKS, &gActiveModels[idx], 1, "Xform Vertices", &gXformMesh[idx]);
	}
	else
	{
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::TooSmall, &mTaskData[idx], kNumOccluderVisTasks, NULL, 0, "TooSmall", &gTooSmall[idx]);
	
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::ActiveModels, &mTaskData[idx], 1, &gTooSmall[idx], 1, "IsActive", &gActiveModels[idx]);

		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::TransformMeshes, &mTaskData[idx], NUM_XFORMVERTS_TASKS, &gActiveModels[idx], 1, "Xform Vertices", &gXformMesh[idx]);
	}

	gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::BinTransformedMeshes, &mTaskData[idx], NUM_XFORMVERTS_TASKS, &gXformMesh[idx], 1, "Bin Meshes", &gBinMesh[idx]);

	gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::SortBins, &mTaskData[idx], 1, &gBinMesh[idx], 1, "BinSort", &gSortBins[idx]);
	
	gTaskMgr.CreateTaskSet(&DepthBufferRasterizerSSEMT::RasterizeBinnedTrianglesToDepthBuffer, &mTaskData[idx], NUM_TILES, &gSortBins[idx], 1, "Raster Tris to DB", &gRasterize[idx]);
}

void DepthBufferRasterizerSSEMT::TransformMeshes(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->TransformMeshes(taskId, taskCount, pTaskData->idx);
}

//------------------------------------------------------------------------------------------------------------
// This function combines the vertices of all the occluder models in the scene and processes the models/meshes 
// that contain the task's triangle range. It trsanform the occluder vertices once every frame
//------------------------------------------------------------------------------------------------------------
void DepthBufferRasterizerSSEMT::TransformMeshes(UINT taskId, UINT taskCount, UINT idx)
{
	UINT verticesPerTask  = mNumVerticesA[idx]/taskCount;
	verticesPerTask		  = (mNumVerticesA[idx] % taskCount) > 0 ? verticesPerTask + 1 : verticesPerTask;
	UINT startIndex		  = taskId * verticesPerTask;

	UINT remainingVerticesPerTask = verticesPerTask;

	// Now, process all of the surfaces that contain this task's triangle range.
	UINT runningVertexCount = 0;
	for(UINT active = 0; active < mNumModelsA[idx]; active++)
    {
		UINT ss = mpModelIndexA[idx][active];
		UINT thisSurfaceVertexCount = mpTransformedModels1[ss].GetNumVertices();
        
        UINT newRunningVertexCount = runningVertexCount + thisSurfaceVertexCount;
        if( newRunningVertexCount < startIndex )
        {
            // We haven't reached the first surface in our range yet.  Skip to the next surface.
            runningVertexCount = newRunningVertexCount;
            continue;
        }

        // If we got this far, then we need to process this surface.
        UINT thisSurfaceStartIndex = max( 0, (int)startIndex - (int)runningVertexCount );
        UINT thisSurfaceEndIndex   = min( thisSurfaceStartIndex + remainingVerticesPerTask, thisSurfaceVertexCount) - 1;

		mpTransformedModels1[ss].TransformMeshes(thisSurfaceStartIndex, thisSurfaceEndIndex, mpCamera[idx], idx);

		remainingVerticesPerTask -= (thisSurfaceEndIndex + 1 - thisSurfaceStartIndex);
        if( remainingVerticesPerTask <= 0 ) break;

		runningVertexCount = newRunningVertexCount;
    }
}

void DepthBufferRasterizerSSEMT::BinTransformedMeshes(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->BinTransformedMeshes(taskId, taskCount, pTaskData->idx);
}

//--------------------------------------------------------------------------------------
// This function combines the triangles of all the occluder models in the scene and processes 
// the models/meshes that contain the task's triangle range. It bins the occluder triangles 
// into tiles once every frame
//--------------------------------------------------------------------------------------
void DepthBufferRasterizerSSEMT::BinTransformedMeshes(UINT taskId, UINT taskCount, UINT idx)
{
	// Reset the bin count.  Note the data layout makes this traversal a bit awkward.
    // We can't just use memset() because the last array index isn't what's varying.
    // However, this should make the real use of this structure go faster.
	for(UINT yy = 0; yy < SCREENH_IN_TILES; yy++)
    {
		UINT offset = YOFFSET1_MT * yy;
        for(UINT xx = 0; xx < SCREENW_IN_TILES; xx++)
        {
			UINT index = offset + (XOFFSET1_MT * xx) + (TOFFSET1_MT * taskId);
            mpNumTrisInBin[idx][index] = 0;
	    }
    }

	// Making sure that the #of Tris in each task (except the last one) is a multiple of 4 
	UINT trianglesPerTask  = (mNumTrianglesA[idx] + taskCount - 1)/taskCount;
	trianglesPerTask      += (trianglesPerTask % SSE) != 0 ? SSE - (trianglesPerTask % SSE) : 0;
	
	UINT startIndex		   = taskId * trianglesPerTask;
	
	UINT remainingTrianglesPerTask = trianglesPerTask;

	// Now, process all of the surfaces that contain this task's triangle range.
	UINT runningTriangleCount = 0;
	for(UINT active = 0; active < mNumModelsA[idx]; active++)
    {
		UINT ss = mpModelIndexA[idx][active];
		UINT thisSurfaceTriangleCount = mpTransformedModels1[ss].GetNumTriangles();
        
        UINT newRunningTriangleCount = runningTriangleCount + thisSurfaceTriangleCount;
        if( newRunningTriangleCount < startIndex )
        {
            // We haven't reached the first surface in our range yet.  Skip to the next surface.
            runningTriangleCount = newRunningTriangleCount;
            continue;
        }

        // If we got this far, then we need to process this surface.
        UINT thisSurfaceStartIndex = max( 0, (int)startIndex - (int)runningTriangleCount );
        UINT thisSurfaceEndIndex   = min( thisSurfaceStartIndex + remainingTrianglesPerTask, thisSurfaceTriangleCount) - 1;

       	mpTransformedModels1[ss].BinTransformedTrianglesMT(taskId, ss, thisSurfaceStartIndex, thisSurfaceEndIndex, mpBin[idx], mpNumTrisInBin[idx], idx);
		remainingTrianglesPerTask -= ( thisSurfaceEndIndex + 1 - thisSurfaceStartIndex);
        if( remainingTrianglesPerTask <= 0 ) break;
				
		runningTriangleCount = newRunningTriangleCount;
    }
}

void DepthBufferRasterizerSSEMT::RasterizeBinnedTrianglesToDepthBuffer(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->RasterizeBinnedTrianglesToDepthBuffer(taskId, pTaskData->idx);
}

//--------------------------------------------------------------------------------------
// This function sorts the tiles in order of decreasing number of triangles; since the
// scheduler starts tasks roughly in order, the idea is to put the "fat tiles" first
// and leave the small jobs for last. This is to avoid the pathological case where a
// relatively big tile gets picked up late (as the other worker threads are about to
// finish) and rendering effectively completes single-threaded.
//--------------------------------------------------------------------------------------
void DepthBufferRasterizerSSEMT::SortBins(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;

	// Initialize sequence in sequential order and compute total number of triangles
	// in the bins for each tile
	UINT tileTotalTris[NUM_TILES];
	for(UINT tile = 0; tile < NUM_TILES; tile++)
	{
		pTaskData->pDBR->mTileSequence[pTaskData->idx][tile] = tile;

		UINT numTris = 0;
		for (UINT bin = 0; bin < NUM_XFORMVERTS_TASKS; bin++)
		{
			numTris += pTaskData->pDBR->mpNumTrisInBin[pTaskData->idx][tile * NUM_XFORMVERTS_TASKS + bin];
		}
		tileTotalTris[tile] = numTris;
	}

	// Sort tiles by number of triangles, decreasing.
	std::sort(pTaskData->pDBR->mTileSequence[pTaskData->idx], pTaskData->pDBR->mTileSequence[pTaskData->idx] + NUM_TILES,
		[&](const UINT a, const UINT b){ return tileTotalTris[a] > tileTotalTris[b]; });
}

//-------------------------------------------------------------------------------
// For each tile go through all the bins and process all the triangles in it.
// Rasterize each triangle to the CPU depth buffer. 
//-------------------------------------------------------------------------------
void DepthBufferRasterizerSSEMT::RasterizeBinnedTrianglesToDepthBuffer(UINT rawTaskId, UINT idx)
{
	UINT taskId = mTileSequence[idx][rawTaskId];
	// Set DAZ and FZ MXCSR bits to flush denormals to zero (i.e., make it faster)
	// Denormal are zero (DAZ) is bit 6 and Flush to zero (FZ) is bit 15. 
	// so to enable the two to have to set bits 6 and 15 which 1000 0000 0100 0000 = 0x8040
	_mm_setcsr( _mm_getcsr() | 0x8040 );

	__m128i colOffset = _mm_setr_epi32(0, 1, 0, 1);
	__m128i rowOffset = _mm_setr_epi32(0, 0, 1, 1);

	float* pDepthBuffer = (float*)mpRenderTargetPixels[idx]; 

	// Based on TaskId determine which tile to process
	UINT screenWidthInTiles = SCREENW/TILE_WIDTH_IN_PIXELS;
    UINT tileX = taskId % screenWidthInTiles;
    UINT tileY = taskId / screenWidthInTiles;

    int tileStartX = tileX * TILE_WIDTH_IN_PIXELS;
	int tileEndX   = min(tileStartX + TILE_WIDTH_IN_PIXELS - 1, SCREENW - 1);
	
	int tileStartY = tileY * TILE_HEIGHT_IN_PIXELS;
	int tileEndY   = min(tileStartY + TILE_HEIGHT_IN_PIXELS - 1, SCREENH - 1);

	ClearDepthTile(tileStartX, tileStartY, tileEndX + 1, tileEndY + 1, idx);

	UINT bin = 0;
	UINT binIndex = 0;
	UINT offset1 = YOFFSET1_MT * tileY + XOFFSET1_MT * tileX;
	UINT offset2 = YOFFSET2_MT * tileY + XOFFSET2_MT * tileX;
	UINT numTrisInBin = mpNumTrisInBin[idx][offset1 + TOFFSET1_MT * bin];

	__m128 gatherBuf[4][2];
	bool done = false;
	bool allBinsEmpty = true;
	mNumRasterizedTris[idx][taskId] = numTrisInBin;
	while(!done)
	{
		// Loop through all the bins and process the 4 binned traingles at a time
		UINT ii;
		int numSimdTris = 0;
		for(ii = 0; ii < SSE; ii++)
		{
			while(numTrisInBin <= 0)
			{
				 // This bin is empty.  Move to next bin.
				if(++bin >= NUM_XFORMVERTS_TASKS)
				{
					break;
				}
				numTrisInBin = mpNumTrisInBin[idx][offset1 + TOFFSET1_MT * bin];
				mNumRasterizedTris[idx][taskId] += numTrisInBin;
				binIndex = 0;
			}
			if(!numTrisInBin)
			{
				 break; // No more tris in the bins
			}
			const BinTriangle *pTri = &mpBin[idx][offset2 + bin * MAX_TRIS_IN_BIN_MT + binIndex];
		    gatherBuf[ii][0] = _mm_castsi128_ps(_mm_loadu_si128((const __m128i *) &pTri->vert[0].xy));
			gatherBuf[ii][1] = _mm_castsi128_ps(_mm_loadl_epi64((const __m128i *) &pTri->Z[1]));
			allBinsEmpty = false;
			numSimdTris++; 

			++binIndex;
			--numTrisInBin;
		}
		done = bin >= NUM_XFORMVERTS_TASKS;
		
		if(allBinsEmpty)
		{
			QueryPerformanceCounter(&mStopTime[idx][taskId]);
			return;
		}

		// use fixed-point only for X and Y.  Avoid work for Z and W.
        __m128i fxPtX[3], fxPtY[3];
		{
			// read vertex data
			__m128 v0 = gatherBuf[0][0];
			__m128 v1 = gatherBuf[1][0];
			__m128 v2 = gatherBuf[2][0];
			__m128 v3 = gatherBuf[3][0];
  			// transpose 
			_MM_TRANSPOSE4_PS(v0, v1, v2, v3);
			
			// Now v0, v1, v2 contain the corresponding verts
			// v3 also contains Z[0] but we don't care here
			fxPtX[0] = _mm_srai_epi32(_mm_slli_epi32(_mm_castps_si128(v0), 16), 16);
			fxPtY[0] = _mm_srai_epi32(_mm_castps_si128(v0), 16);
			fxPtX[1] = _mm_srai_epi32(_mm_slli_epi32(_mm_castps_si128(v1), 16), 16);
			fxPtY[1] = _mm_srai_epi32(_mm_castps_si128(v1), 16);
			fxPtX[2] = _mm_srai_epi32(_mm_slli_epi32(_mm_castps_si128(v2), 16), 16);
			fxPtY[2] = _mm_srai_epi32(_mm_castps_si128(v2), 16);
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

		// Use bounding box traversal strategy to determine whichhttps://maps.google.com/maps?hl=en&tab=ml pixels to rasterize 
		__m128i startX = _mm_and_si128(Max(Min(Min(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(tileStartX)), _mm_set1_epi32(0xFFFFFFFE));
		__m128i endX   = Min(_mm_add_epi32(Max(Max(fxPtX[0], fxPtX[1]), fxPtX[2]), _mm_set1_epi32(1)), _mm_set1_epi32(tileEndX));

		__m128i startY = _mm_and_si128(Max(Min(Min(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(tileStartY)), _mm_set1_epi32(0xFFFFFFFE));
		__m128i endY   = Min(_mm_add_epi32(Max(Max(fxPtY[0], fxPtY[1]), fxPtY[2]), _mm_set1_epi32(1)), _mm_set1_epi32(tileEndY));

        // Now we have 4 triangles set up.  Rasterize them each individually.
        for(int lane=0; lane < numSimdTris; lane++)
        {
			// Extract this triangle's properties from the SIMD versions
            __m128 zz[3];
			zz[0] = _mm_set1_ps(gatherBuf[lane][0].m128_f32[3]);
	        zz[1] = _mm_set1_ps(gatherBuf[lane][1].m128_f32[0]);
            zz[2] = _mm_set1_ps(gatherBuf[lane][1].m128_f32[1]);

			int startXx = startX.m128i_i32[lane];
			int endXx	= endX.m128i_i32[lane];
			int startYy = startY.m128i_i32[lane];
			int endYy	= endY.m128i_i32[lane];
		
			 // Incrementally compute Fab(x, y) for all the pixels inside the bounding box formed by (startX, endX) and (startY, endY) 
			__m128i aa0 = _mm_set1_epi32(A0.m128i_i32[lane]);
			__m128i aa1 = _mm_set1_epi32(A1.m128i_i32[lane]);
			__m128i aa2 = _mm_set1_epi32(A2.m128i_i32[lane]);

			__m128i bb0 = _mm_set1_epi32(B0.m128i_i32[lane]);
			__m128i bb1 = _mm_set1_epi32(B1.m128i_i32[lane]);
			__m128i bb2 = _mm_set1_epi32(B2.m128i_i32[lane]);

			__m128i aa0Inc = _mm_slli_epi32(aa0, 1);
			__m128i aa1Inc = _mm_slli_epi32(aa1, 1);
			__m128i aa2Inc = _mm_slli_epi32(aa2, 1);

			__m128i row, col;

			// Tranverse pixels in 2x2 blocks and store 2x2 pixel quad depthscontiguously in memory ==> 2*X
			// This method provides better perfromance
			int rowIdx = (startYy * SCREENW + 2 * startXx);
			
			col = _mm_add_epi32(colOffset, _mm_set1_epi32(startXx));
			__m128i aa0Col = _mm_mullo_epi32(aa0, col);
			__m128i aa1Col = _mm_mullo_epi32(aa1, col);
			__m128i aa2Col = _mm_mullo_epi32(aa2, col);

			row = _mm_add_epi32(rowOffset, _mm_set1_epi32(startYy));
			__m128i bb0Row = _mm_add_epi32(_mm_mullo_epi32(bb0, row), _mm_set1_epi32(C0.m128i_i32[lane]));
			__m128i bb1Row = _mm_add_epi32(_mm_mullo_epi32(bb1, row), _mm_set1_epi32(C1.m128i_i32[lane]));
			__m128i bb2Row = _mm_add_epi32(_mm_mullo_epi32(bb2, row), _mm_set1_epi32(C2.m128i_i32[lane]));

			__m128i sum0Row = _mm_add_epi32(aa0Col, bb0Row);
			__m128i sum1Row = _mm_add_epi32(aa1Col, bb1Row);
			__m128i sum2Row = _mm_add_epi32(aa2Col, bb2Row);

			__m128i bb0Inc = _mm_slli_epi32(bb0, 1);
			__m128i bb1Inc = _mm_slli_epi32(bb1, 1);
			__m128i bb2Inc = _mm_slli_epi32(bb2, 1);

			__m128 zx = _mm_mul_ps(_mm_cvtepi32_ps(aa1Inc), zz[1]);
			zx = _mm_add_ps(zx, _mm_mul_ps(_mm_cvtepi32_ps(aa2Inc), zz[2]));

			for(int r = startYy; r < endYy; r += 2,
											rowIdx += 2 * SCREENW,
											sum0Row = _mm_add_epi32(sum0Row, bb0Inc),
											sum1Row = _mm_add_epi32(sum1Row, bb1Inc),
											sum2Row = _mm_add_epi32(sum2Row, bb2Inc))
			{
				// Compute barycentric coordinates 
				int index = rowIdx;
				__m128i alpha = sum0Row;
				__m128i beta = sum1Row;
				__m128i gama = sum2Row;

				//Compute barycentric-interpolated depth
				__m128 depth = zz[0];
				depth = _mm_add_ps(depth, _mm_mul_ps(_mm_cvtepi32_ps(beta), zz[1]));
				depth = _mm_add_ps(depth, _mm_mul_ps(_mm_cvtepi32_ps(gama), zz[2]));

				for(int c = startXx; c < endXx; c += 2,
												index += 4,
												alpha = _mm_add_epi32(alpha, aa0Inc),
												beta  = _mm_add_epi32(beta, aa1Inc),
												gama  = _mm_add_epi32(gama, aa2Inc), 
												depth = _mm_add_ps(depth, zx))
				{
					//Test Pixel inside triangle
					__m128i mask = _mm_or_si128(_mm_or_si128(alpha, beta), gama);
					
					//Update depth
					__m128 previousDepthValue = _mm_load_ps(&pDepthBuffer[index]);
					__m128 mergedDepth = _mm_max_ps(depth, previousDepthValue);
					__m128 finaldepth = _mm_blendv_ps(mergedDepth, previousDepthValue, _mm_castsi128_ps(mask));
					_mm_store_ps(&pDepthBuffer[index], finaldepth);
				}//for each column											
			}// for each row
		}// for each triangle
	}// for each set of SIMD# triangles	
	QueryPerformanceCounter(&mStopTime[idx][taskId]);

	// Summarize depth buffer
	CreateCoarseDepth(tileStartX, tileStartY, tileEndX + 1, tileEndY + 1, idx);
}

void DepthBufferRasterizerSSEMT::ComputeR2DBTime(UINT idx)
{
	LARGE_INTEGER stopTime = mStopTime[idx][0];
	for(UINT i = 0; i < NUM_TILES; i++)
	{
		stopTime = stopTime.QuadPart < mStopTime[idx][i].QuadPart ? mStopTime[idx][i] : stopTime;
	}

	mRasterizeTime[mTimeCounter++] = ((double)(stopTime.QuadPart - mStartTime[idx].QuadPart))/((double)glFrequency.QuadPart);
	mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;	
}