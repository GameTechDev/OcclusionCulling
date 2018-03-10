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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "CPUTMath.h"
#include "TaskMgrTBB.h"

// Choose which depth buffer size to use
// If you change depth buffer size please do a rebuilt all 
//#define DP_VERY_VERY_LARGE
#define DP_VERY_LARGE
//#define DP_LARGE
//#define DP_MEDIUM
//#define DP_SMALL

enum SOC_TYPE
{
	SCALAR_TYPE,
	SSE_TYPE,
	AVX_TYPE,
	MASK_AVX_TYPE,
};

extern SOC_TYPE gSOCType;
extern float gOccluderSizeThreshold;
extern float gOccludeeSizeThreshold;
extern UINT  gDepthTestTasks;

extern TASKSETHANDLE gInsideViewFrustum[2];
extern TASKSETHANDLE gTooSmall[2];
extern TASKSETHANDLE gActiveModels[2];
extern TASKSETHANDLE gXformMesh[2];
extern TASKSETHANDLE gBinMesh[2];
extern TASKSETHANDLE gSortBins[2];
extern TASKSETHANDLE gRasterize[2];
extern TASKSETHANDLE gAABBoxDepthTest[2];

extern LARGE_INTEGER glFrequency;

#define PI 3.1415926535f

#if defined(DP_VERY_VERY_LARGE)
    const int SCREENW = 2432;
    const int SCREENH = 1440;

    const int TILE_WIDTH_IN_PIXELS = 304;
    const int TILE_HEIGHT_IN_PIXELS = 360;
#elif defined(DP_VERY_LARGE)
	const int SCREENW = 1920;
	const int SCREENH = 1080;

	const int TILE_WIDTH_IN_PIXELS = 240;
	const int TILE_HEIGHT_IN_PIXELS = 288;														
#elif defined(DP_LARGE)
	const int SCREENW = 1280;
	const int SCREENH = 720;

	const int TILE_WIDTH_IN_PIXELS   = 160;
	const int TILE_HEIGHT_IN_PIXELS  = 192;
#elif defined(DP_MEDIUM)
	const int SCREENW = 640;
	const int SCREENH = 360;

	const int TILE_WIDTH_IN_PIXELS   = 80;
	const int TILE_HEIGHT_IN_PIXELS  = 96;
#elif defined(DP_SMALL)
	const int SCREENW = 320;
	const int SCREENH = 192;

	const int TILE_WIDTH_IN_PIXELS   = 40;
	const int TILE_HEIGHT_IN_PIXELS  = 48;
#endif


const int SCREENW_IN_TILES = (SCREENW + TILE_WIDTH_IN_PIXELS-1)/TILE_WIDTH_IN_PIXELS;
const int SCREENH_IN_TILES = (SCREENH + TILE_HEIGHT_IN_PIXELS-1)/TILE_HEIGHT_IN_PIXELS;

const int NUM_XFORMVERTS_TASKS = 16;

const int NUM_TILES = SCREENW_IN_TILES * SCREENH_IN_TILES;

// depending upon the scene the max #of tris in the bin should be changed.
const int MAX_TRIS_IN_BIN_MT = 1024 * 16;
const int MAX_TRIS_IN_BIN_ST = 1024 * 16;

const int YOFFSET1_ST = SCREENW_IN_TILES;
const int XOFFSET1_ST = 1;

const int YOFFSET2_ST = SCREENW_IN_TILES * MAX_TRIS_IN_BIN_ST;
const int XOFFSET2_ST = MAX_TRIS_IN_BIN_ST;

const int YOFFSET1_MT = SCREENW_IN_TILES;
const int XOFFSET1_MT = 1;
const int TOFFSET1_MT = SCREENW_IN_TILES * SCREENH_IN_TILES;

const int YOFFSET2_MT = SCREENW_IN_TILES * NUM_XFORMVERTS_TASKS * MAX_TRIS_IN_BIN_MT;
const int XOFFSET2_MT = NUM_XFORMVERTS_TASKS * MAX_TRIS_IN_BIN_MT;

const int SSE = 4;
const int AVX = 8;

const int AABB_VERTICES = 8;
const int AABB_INDICES  = 36;
const int AABB_TRIANGLES = 12;

const float4x4 viewportMatrix(
    0.5f*(float)SCREENW,                 0.0f,  0.0f, 0.0f,
                   0.0f, -0.5f*(float)SCREENH,  0.0f, 0.0f,
                   0.0f,                 0.0f,  1.0f, 0.0f,
    0.5f*(float)SCREENW,  0.5f*(float)SCREENH,  0.0f, 1.0f
);

// This needs to be checked for correctness
const float4x4 viewportMatrixMaskedOcclusionCulling(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);

const int OCCLUDER_SETS = 2;
const int OCCLUDEE_SETS = 4;

const int AVG_COUNTER = 10;

const int NUM_DT_TASKS = 50;

const int BLOCK_SIZE = 8;
#endif