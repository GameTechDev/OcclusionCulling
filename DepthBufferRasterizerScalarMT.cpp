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

#include "DepthBufferRasterizerScalarMT.h"
#include"HelperMT.h"

DepthBufferRasterizerScalarMT::DepthBufferRasterizerScalarMT()
	: DepthBufferRasterizerScalar()
{
	int size = SCREENH_IN_TILES * SCREENW_IN_TILES *  NUM_XFORMVERTS_TASKS;
	mpBin[0] = new UINT[size * MAX_TRIS_IN_BIN_MT];
	mpBinModel[0] = new USHORT[size * MAX_TRIS_IN_BIN_MT];
	mpBinMesh[0] = new USHORT[size * MAX_TRIS_IN_BIN_MT];
	mpNumTrisInBin[0] = new USHORT[size];

	mpBin[1] = new UINT[size * MAX_TRIS_IN_BIN_MT];
	mpBinModel[1] = new USHORT[size * MAX_TRIS_IN_BIN_MT];
	mpBinMesh[1] = new USHORT[size * MAX_TRIS_IN_BIN_MT];
	mpNumTrisInBin[1] = new USHORT[size];
}

DepthBufferRasterizerScalarMT::~DepthBufferRasterizerScalarMT()
{
	SAFE_DELETE_ARRAY(mpBin[0]);
	SAFE_DELETE_ARRAY(mpBinModel[0]);
	SAFE_DELETE_ARRAY(mpBinMesh[0]);
	SAFE_DELETE_ARRAY(mpNumTrisInBin[0]);

	SAFE_DELETE_ARRAY(mpBin[1]);
	SAFE_DELETE_ARRAY(mpBinModel[1]);
	SAFE_DELETE_ARRAY(mpBinMesh[1]);
	SAFE_DELETE_ARRAY(mpNumTrisInBin[1]);
}


void DepthBufferRasterizerScalarMT::InsideViewFrustum(VOID *taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->InsideViewFrustum(taskId, taskCount, pTaskData->idx);
}

//------------------------------------------------------------
// * Determine if the occluder model is inside view frustum
//------------------------------------------------------------
void DepthBufferRasterizerScalarMT::InsideViewFrustum(UINT taskId, UINT taskCount, UINT idx)
{
	UINT start, end;
	GetWorkExtent(&start, &end, taskId, taskCount, mNumModels1);

	BoxTestSetupScalar setup;
	setup.Init(mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrix, mpCamera[idx], mOccluderSizeThreshold); 

	for(UINT i = start; i < end; i++)
	{
		mpTransformedModels1[i].InsideViewFrustum(setup, idx);
	}
}

void DepthBufferRasterizerScalarMT::TooSmall(VOID *taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->TooSmall(taskId, taskCount, pTaskData->idx);
}

//------------------------------------------------------------
// * Determine if the occluder model is inside view frustum
//------------------------------------------------------------
void DepthBufferRasterizerScalarMT::TooSmall(UINT taskId, UINT taskCount, UINT idx)
{
	UINT start, end;
	GetWorkExtent(&start, &end, taskId, taskCount, mNumModels1);

	BoxTestSetupScalar setup;
	setup.Init(mpViewMatrix[idx], mpProjMatrix[idx], viewportMatrix, mpCamera[idx], mOccluderSizeThreshold); 

	for(UINT i = start; i < end; i++)
	{
		mpTransformedModels1[i].TooSmall(setup, idx);
	}
}

void DepthBufferRasterizerScalarMT::ActiveModels(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->ActiveModels(taskId, pTaskData->idx);
}

void DepthBufferRasterizerScalarMT::ActiveModels(UINT taskId, UINT idx)
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
void DepthBufferRasterizerScalarMT::TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx)
{
	static const unsigned int kNumOccluderVisTasks = 32;

	mTaskData[idx].idx = idx;
	mTaskData[idx].pDBR = this;

	QueryPerformanceCounter(&mStartTime[idx]);

	mpCamera[idx] = pCamera;
	if(mEnableFCulling)
	{
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::InsideViewFrustum, &mTaskData[idx], kNumOccluderVisTasks, NULL, 0, "Is Visible", &gInsideViewFrustum[idx]);

		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::ActiveModels, &mTaskData[idx], 1, &gInsideViewFrustum[idx], 1, "IsActive", &gActiveModels[idx]);

		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::TransformMeshes, &mTaskData[idx], NUM_XFORMVERTS_TASKS, &gActiveModels[idx], 1, "Xform Vertices", &gXformMesh[idx]);
	}
	else
	{
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::TooSmall, &mTaskData[idx], kNumOccluderVisTasks, NULL, 0, "TooSmall", &gTooSmall[idx]);
	
		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::ActiveModels, &mTaskData[idx], 1, &gTooSmall[idx], 1, "IsActive", &gActiveModels[idx]);

		gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::TransformMeshes, &mTaskData[idx], NUM_XFORMVERTS_TASKS, &gActiveModels[idx], 1, "Xform Vertices", &gXformMesh[idx]);
	}

	gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::BinTransformedMeshes, &mTaskData[idx], NUM_XFORMVERTS_TASKS, &gXformMesh[idx], 1, "Bin Meshes", &gBinMesh[idx]);
	
	gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::SortBins, &mTaskData[idx], 1, &gBinMesh[idx], 1, "BinSort", &gSortBins[idx]);

	gTaskMgr.CreateTaskSet(&DepthBufferRasterizerScalarMT::RasterizeBinnedTrianglesToDepthBuffer, &mTaskData[idx], NUM_TILES, &gSortBins[idx], 1, "Raster Tris to DB", &gRasterize[idx]);	
}

void DepthBufferRasterizerScalarMT::TransformMeshes(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->TransformMeshes(taskId, taskCount, pTaskData->idx);
}

//------------------------------------------------------------------------------------------------------------
// This function combines the vertices of all the occluder models in the scene and processes the models/meshes 
// that contain the task's triangle range. It trsanform the occluder vertices once every frame
//------------------------------------------------------------------------------------------------------------
void DepthBufferRasterizerScalarMT::TransformMeshes(UINT taskId, UINT taskCount, UINT idx)
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

void DepthBufferRasterizerScalarMT::BinTransformedMeshes(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->BinTransformedMeshes(taskId, taskCount, pTaskData->idx);
}

//--------------------------------------------------------------------------------------
// This function combines the triangles of all the occluder models in the scene and processes 
// the models/meshes that contain the task's triangle range. It bins the occluder triangles 
// into tiles once every frame
//--------------------------------------------------------------------------------------
void DepthBufferRasterizerScalarMT::BinTransformedMeshes(UINT taskId, UINT taskCount, UINT idx)
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

       	mpTransformedModels1[ss].BinTransformedTrianglesMT(taskId, ss, thisSurfaceStartIndex, thisSurfaceEndIndex, mpBin[idx], mpBinModel[idx], mpBinMesh[idx], mpNumTrisInBin[idx], idx);

		remainingTrianglesPerTask -= ( thisSurfaceEndIndex + 1 - thisSurfaceStartIndex);
        if( remainingTrianglesPerTask <= 0 ) break;
				
		runningTriangleCount = newRunningTriangleCount;
    }
}

//--------------------------------------------------------------------------------------
// This function sorts the tiles in order of decreasing number of triangles; since the
// scheduler starts tasks roughly in order, the idea is to put the "fat tiles" first
// and leave the small jobs for last. This is to avoid the pathological case where a
// relatively big tile gets picked up late (as the other worker threads are about to
// finish) and rendering effectively completes single-threaded.
//--------------------------------------------------------------------------------------
void DepthBufferRasterizerScalarMT::SortBins(VOID* taskData, INT context, UINT taskId, UINT taskCount)
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


void DepthBufferRasterizerScalarMT::RasterizeBinnedTrianglesToDepthBuffer(VOID* taskData, INT context, UINT taskId, UINT taskCount)
{
	PerTaskData *pTaskData = (PerTaskData*)taskData;
	pTaskData->pDBR->RasterizeBinnedTrianglesToDepthBuffer(taskId, pTaskData->idx);
}

//-------------------------------------------------------------------------------
// For each tile go through all the bins and process all the triangles in it.
// Rasterize each triangle to the CPU depth buffer. 
//-------------------------------------------------------------------------------
void DepthBufferRasterizerScalarMT::RasterizeBinnedTrianglesToDepthBuffer(UINT rawTaskId, UINT idx)
{
	UINT taskId = mTileSequence[idx][rawTaskId];
	float* pDepthBuffer = (float*)mpRenderTargetPixels[idx]; 

	// Based on TaskId determine which tile to process
	UINT screenWidthInTiles = SCREENW/TILE_WIDTH_IN_PIXELS;
    UINT tileX = taskId % screenWidthInTiles;
    UINT tileY = taskId / screenWidthInTiles;

    int tileStartX = tileX * TILE_WIDTH_IN_PIXELS;
	int tileEndX   = min(tileStartX + TILE_WIDTH_IN_PIXELS - 1, SCREENW - 1);
	
	int tileStartY = tileY * TILE_HEIGHT_IN_PIXELS;
	int tileEndY   = min(tileStartY + TILE_HEIGHT_IN_PIXELS - 1, SCREENH - 1);

	UINT bin = 0;
	UINT binIndex = 0;
	UINT offset1 = YOFFSET1_MT * tileY + XOFFSET1_MT * tileX;
	UINT offset2 = YOFFSET2_MT * tileY + XOFFSET2_MT * tileX;
	UINT numTrisInBin = mpNumTrisInBin[idx][offset1 + TOFFSET1_MT * bin];

	ClearDepthTile(tileStartX, tileStartY, tileEndX + 1, tileEndY + 1, idx);

	float4 xformedPos[3];
	bool done = false;
	bool allBinsEmpty = true;
	mNumRasterizedTris[idx][taskId] = numTrisInBin;

	while(!done)
	{
		// Loop through all the bins and process the binned traingles
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
		USHORT modelId = mpBinModel[idx][offset2 + bin * MAX_TRIS_IN_BIN_MT + binIndex];
		USHORT meshId = mpBinMesh[idx][offset2 + bin * MAX_TRIS_IN_BIN_MT + binIndex];
		UINT triIdx = mpBin[idx][offset2 + bin * MAX_TRIS_IN_BIN_MT + binIndex];
		mpTransformedModels1[modelId].Gather((float*)xformedPos, meshId, triIdx, idx);
		allBinsEmpty = false;
		
		++binIndex;
		--numTrisInBin;
		
		done = bin >= NUM_XFORMVERTS_TASKS;
		
		if(allBinsEmpty)
		{
			QueryPerformanceCounter(&mStopTime[idx][taskId]);
			return;
		}

		// use fixed-point only for X and Y.  Avoid work for Z and W.
		int fxPtX[3], fxPtY[3];
		float Z[3];
		for(UINT i = 0; i < 3; i++)
		{
			fxPtX[i] = (int)(xformedPos[i].x + 0.5);
			fxPtY[i] = (int)(xformedPos[i].y + 0.5);
			Z[i] = xformedPos[i].z;
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
		int startX = max(min(min(fxPtX[0], fxPtX[1]), fxPtX[2]), tileStartX) & int(0xFFFFFFFE);
		int endX   = min(max(max(fxPtX[0], fxPtX[1]), fxPtX[2]), tileEndX+1);

		int startY = max(min(min(fxPtY[0], fxPtY[1]), fxPtY[2]), tileStartY) & int(0xFFFFFFFE);
		int endY   = min(max(max(fxPtY[0], fxPtY[1]), fxPtY[2]), tileEndY+1);

		int rowIdx = (startY * SCREENW + startX);
		int col = startX;
		int row = startY;
		
		// Incrementally compute Fab(x, y) for all the pixels inside the bounding box formed by (startX, endX) and (startY, endY) 
		int alpha0 = (A0 * col) + (B0 * row) + C0;
		int beta0 = (A1 * col) + (B1 * row) + C1;
		int gama0 = (A2 * col) + (B2 * row) + C2;

		float zx = A1 * Z[1] + A2 * Z[2];
				
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
			
			for(int c = startX; c < endX; c++,
   										  index++,
										  alpha += A0,
										  beta  += A1,
										  gama  += A2,
										  depth += zx)
			{
				//Test Pixel inside triangle
				int mask = alpha | beta | gama;
					
				float previousDepthValue = pDepthBuffer[index];
				float mergedDepth = max(depth, previousDepthValue);				
				float finaldepth = mask < 0 ? previousDepthValue : mergedDepth;
				
				pDepthBuffer[index] = finaldepth;
			}//for each column											
		}// for each row
	}// for each triangle
	QueryPerformanceCounter(&mStopTime[idx][taskId]);
}


void DepthBufferRasterizerScalarMT::ComputeR2DBTime(UINT idx)
{
	LARGE_INTEGER stopTime = mStopTime[idx][0];
	for(UINT i = 0; i < NUM_TILES; i++)
	{
		stopTime = stopTime.QuadPart < mStopTime[idx][i].QuadPart ? mStopTime[idx][i] : stopTime;
	}

	mRasterizeTime[mTimeCounter++] = ((double)(stopTime.QuadPart - mStartTime[idx].QuadPart))/((double)glFrequency.QuadPart);
	mTimeCounter = mTimeCounter >= AVG_COUNTER ? 0 : mTimeCounter;	
}