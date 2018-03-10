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
#include "OcclusionCulling.h"
#include "CPUTRenderTarget.h"
#include "CPUTTextureDX11.h"
#include "MaskedOcclusionCulling\MaskedOcclusionCulling.h"
#include "MaskedOcclusionCulling\CullingThreadpool.h"

const UINT SHADOW_WIDTH_HEIGHT = 256;

// set file to open
cString g_OpenFilePath;
cString g_OpenShaderPath;
cString g_OpenFileName;

extern float3 gLightDir;
extern char *gpDefaultShaderSource;

float gFarClipDistance = 2000.0f;

MaskedOcclusionCulling *gMaskedOcclusionCulling = nullptr;
CullingThreadpool *gMaskedOcclusionCullingThreadpool = nullptr;

SOC_TYPE gSOCType = SSE_TYPE; // MASK_AVX_TYPE; 
float gOccluderSizeThreshold = 1.5f;
float gOccludeeSizeThreshold = 0.01f;
UINT  gDepthTestTasks		 = 20;
TASKSETHANDLE gInsideViewFrustum[2]	= {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};
TASKSETHANDLE gTooSmall[2]			= {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};
TASKSETHANDLE gActiveModels[2]      = {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};
TASKSETHANDLE gXformMesh[2]			= {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};
TASKSETHANDLE gBinMesh[2]			= {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};
TASKSETHANDLE gSortBins[2]			= {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};
TASKSETHANDLE gRasterize[2]			= {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};
TASKSETHANDLE gAABBoxDepthTest[2]   = {TASKSETHANDLE_INVALID, TASKSETHANDLE_INVALID};

LARGE_INTEGER glFrequency; 


MySample::MySample() :
	mpCameraController(NULL),
	mpDebugSprite(NULL),
	mpShadowCameraSet(NULL),
	mpShadowRenderTarget(NULL),
	mpFPSCounter(NULL),
	mpTypeDropDown(NULL),
	mpOccludersText(NULL),
	mpNumOccludersText(NULL),
	mpOccludersR2DBText(NULL),
	mpOccluderTrisText(NULL),
	mpOccluderRasterizedTrisText(NULL),
	mpRasterizeTimeText(NULL),
	mpOccluderSizeSlider(NULL),
	mpOccludeesText(NULL),
	mpNumOccludeesText(NULL),
	mpCulledText(NULL),
	mpVisibleText(NULL),
	mpOccludeeTrisText(NULL),
	mpCulledTrisText(NULL),
	mpVisibleTrisText(NULL),
	mpDepthTestTimeText(NULL),
    mpSOCDepthResolutionText(NULL),
	mpOccludeeSizeSlider(NULL),
	mpTotalCullTimeText(NULL),
	mpCullingCheckBox(NULL),
	mpFCullingCheckBox(NULL),
	mpDBCheckBox(NULL),
	mpBBCheckBox(NULL),
	mpTasksCheckBox(NULL),
	mpVsyncCheckBox(NULL),
	mpPipelineCheckBox(NULL),
//    mpOcclusionMatchGPUResolution(NULL),
	mpDrawCallsText(NULL),
	mpDepthTestTaskSlider(NULL),
	mpGPUDepthBuf(NULL),
	mSOCType(gSOCType),
	mNumOccluders(0),
	mNumOccludersR2DB(0),
	mNumOccluderTris(0),
	mNumOccluderRasterizedTris(0),
	mRasterizeTime(0.0),
	mOccluderSizeThreshold(gOccluderSizeThreshold),
	mNumCulled(0),
	mNumVisible(0),
	mNumOccludeeTris(0),
	mNumOccludeeCulledTris(0),
	mNumOccludeeVisibleTris(0),
	mDepthTestTime(0.0),
	mOccludeeSizeThreshold(gOccludeeSizeThreshold),
	mTotalCullTime(0.0),
	mEnableCulling(true),
	mEnableFCulling(true),
	mViewDepthBuffer(false),
	mViewBoundingBox(false),
	mEnableTasks(true),
	mPipeline(false),
	mNumDrawCalls(0),
	mNumDepthTestTasks(gDepthTestTasks),
	mCurrIdx(0),
	mPrevIdx(1),
	mFirstFrame(true)
//    mOcclusionMatchGPUResolution(true)
{
	mpCPURenderTargetScalar[0] = NULL;
	mpCPURenderTargetScalar[1] = NULL;

	mpCPURenderTargetSSE[0] = NULL;
	mpCPURenderTargetSSE[1] = NULL;

	mpCPURenderTargetAVX[0] = NULL;
	mpCPURenderTargetAVX[1] = NULL;

	mpCPURenderTarget[0] = NULL;
	mpCPURenderTarget[1] = NULL;

	mpCPUSRVScalar[0] = mpCPUSRVScalar[1] = NULL;
	mpCPUSRVSSE[0] = mpCPUSRVSSE[1] = NULL;
	mpCPUSRVAVX[0] = mpCPUSRVAVX[1] = NULL;
	mpCPUSRV[0] = mpCPUSRV[1] = NULL;

	for (UINT i = 0; i < OCCLUDER_SETS; i++)
	{
		mpAssetSetDBR[i] = NULL;
	}

	for (UINT i = 0; i < OCCLUDEE_SETS; i++)
	{
		mpAssetSetAABB[i] = NULL;
	}

	mpAssetSetSky = NULL;
	mpCPUDepthBuf[0] = mpCPUDepthBuf[1] = NULL;
	mpShowDepthBufMtrlScalar = mpShowDepthBufMtrlSSE = mpShowDepthBufMtrlAVX = mpShowDepthBufMtrl = NULL;

    memset( &mTotalCullTimeHistories, 0, sizeof( mTotalCullTimeHistories ) );
    mTotalCullTimeLastIndex = 0;
    mTotalCullTimeAvg = 0.0;
}

MySample::~MySample()
{
	// Note: these two are defined in the base.  We release them because we addref them.
	SAFE_RELEASE(mpCamera);
	SAFE_RELEASE(mpShadowCamera);

	_aligned_free(mpCPUDepthBuf[0]);
	_aligned_free(mpCPUDepthBuf[1]);
	_aligned_free(mpGPUDepthBuf);
	SAFE_RELEASE(mpCPURenderTargetScalar[0]);
	SAFE_RELEASE(mpCPURenderTargetScalar[1]);
	SAFE_RELEASE(mpCPURenderTargetSSE[0]);
	SAFE_RELEASE(mpCPURenderTargetSSE[1]);
	SAFE_RELEASE(mpCPURenderTargetAVX[0]);
	SAFE_RELEASE(mpCPURenderTargetAVX[1]);

	SAFE_RELEASE(mpCPUSRVScalar[0]);
	SAFE_RELEASE(mpCPUSRVScalar[1]);
	SAFE_RELEASE(mpCPUSRVSSE[0]);
	SAFE_RELEASE(mpCPUSRVSSE[1]);
	SAFE_RELEASE(mpCPUSRVAVX[0]);
	SAFE_RELEASE(mpCPUSRVAVX[1]);

	SAFE_DELETE(mpDBR);
	SAFE_DELETE(mpAABB);

	for (UINT i = 0; i < OCCLUDER_SETS; i++)
	{
		SAFE_RELEASE(mpAssetSetDBR[i]);
	}
	SAFE_RELEASE(mpAssetSetAABB[0]);
	SAFE_RELEASE(mpAssetSetAABB[1]);
	SAFE_RELEASE(mpAssetSetSky);

	SAFE_DELETE(mpCameraController);
	SAFE_DELETE(mpDebugSprite);
	SAFE_RELEASE(mpShadowCameraSet);
	SAFE_DELETE(mpShadowRenderTarget);

	SAFE_RELEASE(mpShowDepthBufMtrlScalar);
	SAFE_RELEASE(mpShowDepthBufMtrlSSE);
	SAFE_RELEASE(mpShowDepthBufMtrlAVX);

	CPUTModel::ReleaseStaticResources();
    
    delete gMaskedOcclusionCullingThreadpool;
    gMaskedOcclusionCullingThreadpool = nullptr;
    MaskedOcclusionCulling::Destroy( gMaskedOcclusionCulling );
    gMaskedOcclusionCulling = nullptr;
}

void MySample::SetupOcclusionCullingObjects()
{
	if ((mSOCType == SCALAR_TYPE) && !mEnableTasks)
	{
		mpDBRScalarST = new DepthBufferRasterizerScalarST;
		mpDBR = mpDBRScalarST;

		mpAABBScalarST = new AABBoxRasterizerScalarST;
		mpAABB = mpAABBScalarST;
	}
	else if ((mSOCType == SCALAR_TYPE) && mEnableTasks)
	{
		mpDBRScalarMT = new DepthBufferRasterizerScalarMT;
		mpDBR = mpDBRScalarMT;

		mpAABBScalarMT = new AABBoxRasterizerScalarMT;
		mpAABB = mpAABBScalarMT;
	}
	else if ((mSOCType == SSE_TYPE) && !mEnableTasks)
	{
		mpDBRSSEST = new DepthBufferRasterizerSSEST;
		mpDBR = mpDBRSSEST;

		mpAABBSSEST = new AABBoxRasterizerSSEST;
		mpAABB = mpAABBSSEST;
	}
	else if ((mSOCType == SSE_TYPE) && mEnableTasks)
	{
		mpDBRSSEMT = new DepthBufferRasterizerSSEMT;
		mpDBR = mpDBRSSEMT;

		mpAABBSSEMT = new AABBoxRasterizerSSEMT;
		mpAABB = mpAABBSSEMT;
	}
	else if ((mSOCType == AVX_TYPE) && !mEnableTasks)
	{
		mpDBRAVXST = new DepthBufferRasterizerAVXST;
		mpDBR = mpDBRAVXST;

		mpAABBAVXST = new AABBoxRasterizerAVXST;
		mpAABB = mpAABBAVXST;
	}
	else if ((mSOCType == AVX_TYPE) && mEnableTasks)
	{
		mpDBRAVXMT = new DepthBufferRasterizerAVXMT;
		mpDBR = mpDBRAVXMT;

		mpAABBAVXMT = new AABBoxRasterizerAVXMT;
		mpAABB = mpAABBAVXMT;
	}
	else if (mSOCType == MASK_AVX_TYPE && !mEnableTasks )
	{
		mpDBMRAVXST = new DepthBufferMaskedRasterizerAVXST(gMaskedOcclusionCulling);
		mpDBR = mpDBMRAVXST;

		mpAABBMAVXST = new AABBoxMaskedRasterizerAVXST(gMaskedOcclusionCulling);
		mpAABB = mpAABBMAVXST;
	}
    else if( mSOCType == MASK_AVX_TYPE && mEnableTasks )
    {
//        assert( false ); // not yet implemented
        mpDBMRAVXMT = new DepthBufferMaskedRasterizerAVXMT( gMaskedOcclusionCulling, gMaskedOcclusionCullingThreadpool );
        mpDBR = mpDBMRAVXMT;

        mpAABBMAVXMT = new AABBoxMaskedRasterizerAVXMT( gMaskedOcclusionCulling, gMaskedOcclusionCullingThreadpool, mpDBMRAVXMT );
        mpAABB = mpAABBMAVXMT;
    }
}

static unsigned int GetOptimalNumberOfThreads( );

// Handle OnCreation events
//-----------------------------------------------------------------------------
void MySample::Create()
{    
	// Create occlusion culling resources
	gMaskedOcclusionCulling = MaskedOcclusionCulling::Create();
    gMaskedOcclusionCullingThreadpool = new CullingThreadpool( GetOptimalNumberOfThreads(), 10, 6, 128 );
    gMaskedOcclusionCullingThreadpool->SetBuffer( gMaskedOcclusionCulling );

    // Print which version (instruction set) is being used
    MaskedOcclusionCulling::Implementation implementation = gMaskedOcclusionCulling->GetImplementation( );
    switch( implementation ) {
    case MaskedOcclusionCulling::SSE2: printf( "Using SSE2 version\n" ); break;
    case MaskedOcclusionCulling::SSE41: printf( "Using SSE41 version\n" ); break;
    case MaskedOcclusionCulling::AVX2: printf( "Using AVX2 version\n" ); break;
    }

	//gMaskedOcclusionCulling->SetResolution(SCREENW, SCREENH);
    gMaskedOcclusionCullingThreadpool->SetResolution(SCREENW, SCREENH);
	SetupOcclusionCullingObjects();
	
	CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    gLightDir.normalize();

    // TODO: read from cmd line, using these as defaults
    //pAssetLibrary->SetMediaDirectoryName(    _L("Media"));

    CPUTGuiControllerDX11 *pGUI = CPUTGetGuiController();

    // create some controls
	CPUTButton     *pButton = NULL;
    pGUI->CreateButton(_L("Fullscreen"), ID_FULLSCREEN_BUTTON, ID_MAIN_PANEL, &pButton);
	pGUI->CreateDropdown( L"Rasterizer Technique: SCALAR", ID_RASTERIZE_TYPE, ID_MAIN_PANEL, &mpTypeDropDown);
    mpTypeDropDown->AddSelectionItem( L"Rasterizer Technique: SSE" );
	if (CanUseIntelCore4thGenFeatures())
	{
		mpTypeDropDown->AddSelectionItem(L"Rasterizer Technique: AVX");
		mpTypeDropDown->AddSelectionItem(L"Rasterizer Technique: MOC");
	}
	mpTypeDropDown->SetSelectedItem(mSOCType + 1);
   
	wchar_t string[CPUT_MAX_STRING_LENGTH];
    pGUI->CreateText(    _L("Occluders                                              \t"), ID_OCCLUDERS, ID_MAIN_PANEL, &mpOccludersText);
	
	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of Models: \t%d"), mNumOccluders);
	pGUI->CreateText(string, ID_NUM_OCCLUDERS, ID_MAIN_PANEL, &mpNumOccludersText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tDepth rasterized models: %d"), mNumOccludersR2DB);
	pGUI->CreateText(string, ID_NUM_OCCLUDERS_RASTERIZED_TO_DB, ID_MAIN_PANEL, &mpOccludersR2DBText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of tris: \t\t%d"), mNumOccluderTris);
	pGUI->CreateText(string, ID_NUM_OCCLUDER_TRIS, ID_MAIN_PANEL, &mpOccluderTrisText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tDepth rasterized tris: \t%d"), mNumOccluderRasterizedTris);
	pGUI->CreateText(string, ID_NUM_OCCLUDER_RASTERIZED_TRIS, ID_MAIN_PANEL, &mpOccluderRasterizedTrisText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tDepth raterizer time: \t%f"), mRasterizeTime);
	pGUI->CreateText(string, ID_RASTERIZE_TIME, ID_MAIN_PANEL, &mpRasterizeTimeText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Occluder Size Threshold: %0.4f"), mOccluderSizeThreshold);
	pGUI->CreateSlider(string, ID_OCCLUDER_SIZE, ID_MAIN_PANEL, &mpOccluderSizeSlider);
	mpOccluderSizeSlider->SetScale(0, 5.0, 51);
	mpOccluderSizeSlider->SetValue(mOccluderSizeThreshold);
	mpOccluderSizeSlider->SetTickDrawing(false);
    
	pGUI->CreateText(_L("Occludees                                              \t"), ID_OCCLUDEES, ID_MAIN_PANEL, &mpOccludeesText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of Models: \t%d"), mNumOccludees);
	pGUI->CreateText(string, ID_NUM_OCCLUDEES, ID_MAIN_PANEL, &mpNumOccludeesText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tModels culled: \t%d"), mNumCulled);
	pGUI->CreateText(string, ID_NUM_CULLED, ID_MAIN_PANEL, &mpCulledText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tModels visible: \t%d"), mNumVisible);
	pGUI->CreateText(string, ID_NUM_VISIBLE, ID_MAIN_PANEL, &mpVisibleText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of tris: \t%d"), (int)mNumOccludeeTris);
	pGUI->CreateText(string, ID_NUM_OCCLUDEE_TRIS, ID_MAIN_PANEL, &mpOccludeeTrisText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tCulled Triangles: \t%d"), (int)mNumOccludeeCulledTris);
	pGUI->CreateText(string, ID_NUM_OCCLUDEE_CULLED_TRIS, ID_MAIN_PANEL, &mpCulledTrisText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tVisible Triangles: \t%d"), (int)mNumOccludeeVisibleTris);
	pGUI->CreateText(string, ID_NUM_OCCLUDEE_VISIBLE_TRIS, ID_MAIN_PANEL, &mpVisibleTrisText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tVisible Triangles: \t%0.2f ms"), mDepthTestTime);
	pGUI->CreateText(string, ID_DEPTHTEST_TIME, ID_MAIN_PANEL, &mpDepthTestTimeText);

    swprintf_s( &string[0], CPUT_MAX_STRING_LENGTH, _L( "\tSOC Depth Resolution: \t%d x %d"), 128, 64);
    pGUI->CreateText( string, ID_SOCDEPTHRESOLUTIONTEXT, ID_MAIN_PANEL, &mpSOCDepthResolutionText);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Occludee Size Threshold: %0.4f"), mOccludeeSizeThreshold);
	pGUI->CreateSlider(string, ID_OCCLUDEE_SIZE, ID_MAIN_PANEL, &mpOccludeeSizeSlider);
	mpOccludeeSizeSlider->SetScale(0, 0.1f, 41);
	mpOccludeeSizeSlider->SetValue(mOccludeeSizeThreshold);
	mpOccludeeSizeSlider->SetTickDrawing(false);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Total Cull time: %0.2f"), mTotalCullTime);
	pGUI->CreateText(string, ID_TOTAL_CULL_TIME, ID_MAIN_PANEL, &mpTotalCullTimeText);

	pGUI->CreateCheckbox(_L("Depth Test Culling"),  ID_ENABLE_CULLING, ID_MAIN_PANEL, &mpCullingCheckBox);
	pGUI->CreateCheckbox(_L("Frustum Culling"),  ID_ENABLE_FCULLING, ID_MAIN_PANEL, &mpFCullingCheckBox);
	pGUI->CreateCheckbox(_L("View Depth Buffer"),  ID_DEPTH_BUFFER_VISIBLE, ID_MAIN_PANEL, &mpDBCheckBox);
	pGUI->CreateCheckbox(_L("View Bounding Box"),  ID_BOUNDING_BOX_VISIBLE, ID_MAIN_PANEL, &mpBBCheckBox);
	pGUI->CreateCheckbox(_L("Multithreaded Culling"), ID_ENABLE_TASKS, ID_MAIN_PANEL, &mpTasksCheckBox);
	pGUI->CreateCheckbox(_L("Vsync"), ID_VSYNC_ON_OFF, ID_MAIN_PANEL, &mpVsyncCheckBox);
	pGUI->CreateCheckbox(_L("Pipeline"), ID_PIPELINE, ID_MAIN_PANEL, &mpPipelineCheckBox);

    //pGUI->CreateCheckbox( _L( "Match Occlusion to Framebuffer Resolution" ), ID_OCCLUSIONMATCHGPURESOLUTION, ID_MAIN_PANEL, &mpOcclusionMatchGPUResolution );
    

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Number of draw calls: \t%d"), mNumDrawCalls);
	pGUI->CreateText(string, ID_NUM_DRAW_CALLS, ID_MAIN_PANEL, &mpDrawCallsText),
	
	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Depth Test Tasks: \t\t%d"), mNumDepthTestTasks);
    pGUI->CreateSlider(string, ID_DEPTH_TEST_TASKS, ID_MAIN_PANEL, &mpDepthTestTaskSlider);
	mpDepthTestTaskSlider->SetScale(1, (float)NUM_DT_TASKS, 11);
	mpDepthTestTaskSlider->SetValue((float)mNumDepthTestTasks);
	mpDepthTestTaskSlider->SetTickDrawing(false);
	mpAABB->SetDepthTestTasks(mNumDepthTestTasks);

    //
    // Create Static text
    //
    pGUI->CreateText( _L("F1 for Help"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("[Escape] to quit application"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("A,S,D,F - move camera position"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("Q - camera position up"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("E - camera position down"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("mouse + right click - camera look location"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
	pGUI->CreateText( _L("size thresholds : computed using screen space metris"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);

    pGUI->SetActivePanel(ID_MAIN_PANEL);
    pGUI->DrawFPS(true);

    // Add our programatic (and global) material parameters
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerFrameValues"), _L("$cbPerFrameValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerModelValues"), _L("$cbPerModelValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("_Shadow"), _L("$shadow_depth") );

	// Creating a render target to view the CPU rasterized depth buffer
	// Pad the buffer to 32 byte to support AVX2 data read/write
	mpCPUDepthBuf[0] = (char*)_aligned_malloc(sizeof(char) * ((SCREENW*SCREENH * 4 + 31) & 0xFFFFFFE0), 32);
	mpCPUDepthBuf[1] = (char*)_aligned_malloc(sizeof(char) * ((SCREENW*SCREENH * 4 + 31) & 0xFFFFFFE0), 32);
	mpGPUDepthBuf = (char*)_aligned_malloc(sizeof(char) * ((SCREENW*SCREENH * 4 + 31) & 0xFFFFFFE0), 32);

	CD3D11_TEXTURE2D_DESC cpuRenderTargetDescSSE
	(
		DXGI_FORMAT_R8G8B8A8_UNORM,
        SCREENW * 2, // TODO: round up to full tile sizes
        SCREENH / 2,
        1, // Array Size
        1, // MIP Levels
		D3D11_BIND_SHADER_RESOURCE,
        D3D11_USAGE_DEFAULT,
		0
    );
	HRESULT hr;
	hr = mpD3dDevice->CreateTexture2D(&cpuRenderTargetDescSSE, NULL, &mpCPURenderTargetSSE[0]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating render target."));

	hr = mpD3dDevice->CreateTexture2D(&cpuRenderTargetDescSSE, NULL, &mpCPURenderTargetSSE[1]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating render target."));

	hr = mpD3dDevice->CreateShaderResourceView(mpCPURenderTargetSSE[0], NULL, &mpCPUSRVSSE[0]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating shader resource view."));

	hr = mpD3dDevice->CreateShaderResourceView(mpCPURenderTargetSSE[1], NULL, &mpCPUSRVSSE[1]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating shader resource view."));

	// Corresponding texture object
	CPUTTextureDX11 *pDummyTex0 = new CPUTTextureDX11;
	pDummyTex0->SetTextureAndShaderResourceView(mpCPURenderTargetSSE[0], mpCPUSRVSSE[0]);
	pAssetLibrary->AddTexture( _L("$depthbuf_tex_SSE"), pDummyTex0 );
	SAFE_RELEASE(pDummyTex0);

	CPUTTextureDX11 *pDummyTex1 = new CPUTTextureDX11;
	pDummyTex1->SetTextureAndShaderResourceView(mpCPURenderTargetSSE[1], mpCPUSRVSSE[1]);
	pAssetLibrary->AddTexture( _L("$depthbuf_tex_SSE"), pDummyTex1 );
	SAFE_RELEASE(pDummyTex1);

	CD3D11_TEXTURE2D_DESC cpuRenderTargetDescScalar
	(
		DXGI_FORMAT_R8G8B8A8_UNORM,
        SCREENW, // TODO: round up to full tile sizes
        SCREENH,
        1, // Array Size
        1, // MIP Levels
		D3D11_BIND_SHADER_RESOURCE,
        D3D11_USAGE_DEFAULT,
		0
    );
	hr = mpD3dDevice->CreateTexture2D(&cpuRenderTargetDescScalar, NULL, &mpCPURenderTargetScalar[0]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating render target."));

	hr = mpD3dDevice->CreateTexture2D(&cpuRenderTargetDescScalar, NULL, &mpCPURenderTargetScalar[1]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating render target."));

	hr = mpD3dDevice->CreateShaderResourceView(mpCPURenderTargetScalar[0], NULL, &mpCPUSRVScalar[0]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating shader resource view."));

	hr = mpD3dDevice->CreateShaderResourceView(mpCPURenderTargetScalar[1], NULL, &mpCPUSRVScalar[1]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating shader resource view."));

	// Corresponding texture object
	CPUTTextureDX11 *pDummyTex2 = new CPUTTextureDX11;
	pDummyTex2->SetTextureAndShaderResourceView(mpCPURenderTargetScalar[0], mpCPUSRVScalar[0]);
	pAssetLibrary->AddTexture( _L("$depthbuf_tex_Scalar"), pDummyTex2 );
	SAFE_RELEASE(pDummyTex2);

	CPUTTextureDX11 *pDummyTex3 = new CPUTTextureDX11;
	pDummyTex3->SetTextureAndShaderResourceView(mpCPURenderTargetScalar[1], mpCPUSRVScalar[1]);
	pAssetLibrary->AddTexture( _L("$depthbuf_tex_Scalar"), pDummyTex3 );
	SAFE_RELEASE(pDummyTex3);

	CD3D11_TEXTURE2D_DESC cpuRenderTargetDescAVX
		(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		SCREENW * 2, // TODO: round up to full tile sizes
		SCREENH / 2,
		1, // Array Size
		1, // MIP Levels
		D3D11_BIND_SHADER_RESOURCE,
		D3D11_USAGE_DEFAULT,
		0
		);
	hr = mpD3dDevice->CreateTexture2D(&cpuRenderTargetDescAVX, NULL, &mpCPURenderTargetAVX[0]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating render target."));

	hr = mpD3dDevice->CreateTexture2D(&cpuRenderTargetDescAVX, NULL, &mpCPURenderTargetAVX[1]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating render target."));

	hr = mpD3dDevice->CreateShaderResourceView(mpCPURenderTargetAVX[0], NULL, &mpCPUSRVAVX[0]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating shader resource view."));

	hr = mpD3dDevice->CreateShaderResourceView(mpCPURenderTargetAVX[1], NULL, &mpCPUSRVAVX[1]);
	ASSERT(SUCCEEDED(hr), _L("Failed creating shader resource view."));

	// Corresponding texture object
	CPUTTextureDX11 *pDummyTex4 = new CPUTTextureDX11;
	pDummyTex4->SetTextureAndShaderResourceView(mpCPURenderTargetAVX[0], mpCPUSRVAVX[0]);
	pAssetLibrary->AddTexture(_L("$depthbuf_tex_AVX"), pDummyTex4);
	SAFE_RELEASE(pDummyTex4);

	CPUTTextureDX11 *pDummyTex5 = new CPUTTextureDX11;
	pDummyTex5->SetTextureAndShaderResourceView(mpCPURenderTargetAVX[1], mpCPUSRVAVX[1]);
	pAssetLibrary->AddTexture(_L("$depthbuf_tex_AVX"), pDummyTex5);
	SAFE_RELEASE(pDummyTex5);

	// Create default shaders
    CPUTPixelShaderDX11  *pPS       = CPUTPixelShaderDX11::CreatePixelShaderFromMemory(            _L("$DefaultShader"), CPUT_DX11::mpD3dDevice,          _L("PSMain"), _L("ps_4_0"), gpDefaultShaderSource );
    CPUTPixelShaderDX11  *pPSNoTex  = CPUTPixelShaderDX11::CreatePixelShaderFromMemory(   _L("$DefaultShaderNoTexture"), CPUT_DX11::mpD3dDevice, _L("PSMainNoTexture"), _L("ps_4_0"), gpDefaultShaderSource );
    CPUTVertexShaderDX11 *pVS       = CPUTVertexShaderDX11::CreateVertexShaderFromMemory(          _L("$DefaultShader"), CPUT_DX11::mpD3dDevice,          _L("VSMain"), _L("vs_4_0"), gpDefaultShaderSource );
    CPUTVertexShaderDX11 *pVSNoTex  = CPUTVertexShaderDX11::CreateVertexShaderFromMemory( _L("$DefaultShaderNoTexture"), CPUT_DX11::mpD3dDevice, _L("VSMainNoTexture"), _L("vs_4_0"), gpDefaultShaderSource );

    // We just want to create them, which adds them to the library.  We don't need them any more so release them, leaving refCount at 1 (only library owns a ref)
    SAFE_RELEASE(pPS);
    SAFE_RELEASE(pPSNoTex);
    SAFE_RELEASE(pVS);
    SAFE_RELEASE(pVSNoTex);

    // load shadow casting material+sprite object
	cString ExecutableDirectory;
	CPUTOSServices::GetOSServices()->GetWorkingDirectory(&ExecutableDirectory);
	pAssetLibrary->SetMediaDirectoryName(ExecutableDirectory + _L("\\Media\\"));

    mpShadowRenderTarget = new CPUTRenderTargetDepth();
    mpShadowRenderTarget->CreateRenderTarget( cString(_L("$shadow_depth")), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT );

    mpDebugSprite = new CPUTSprite();
    mpDebugSprite->CreateSprite( -1.0f, -1.0f, 0.5f, 0.5f, _L("Sprite") );

	int width, height;
    CPUTOSServices::GetOSServices()->GetClientDimensions(&width, &height);

	// Depth buffer visualization material
	mpShowDepthBufMtrlScalar = (CPUTMaterialDX11*)CPUTAssetLibraryDX11::GetAssetLibrary()->GetMaterial( _L("showDepthBufScalar"));
	mpShowDepthBufMtrlSSE = (CPUTMaterialDX11*)CPUTAssetLibraryDX11::GetAssetLibrary()->GetMaterial( _L("showDepthBufSSE"));
	mpShowDepthBufMtrlAVX = (CPUTMaterialDX11*)CPUTAssetLibraryDX11::GetAssetLibrary()->GetMaterial(_L("showDepthBufAVX"));
		
	if(mSOCType == SCALAR_TYPE)
	{
		mpCPURenderTarget[0] = mpCPURenderTargetScalar[0];
		mpCPURenderTarget[1] = mpCPURenderTargetScalar[1];
		mpCPUSRV[0]          = mpCPUSRVScalar[0];
		mpCPUSRV[1]          = mpCPUSRVScalar[1];
		mpShowDepthBufMtrl   = mpShowDepthBufMtrlScalar;
		rowPitch			 = SCREENW * 4;
	}
	else if (mSOCType == SSE_TYPE)
	{
		mpCPURenderTarget[0] = mpCPURenderTargetSSE[0];
		mpCPURenderTarget[1] = mpCPURenderTargetSSE[1];
		mpCPUSRV[0]          = mpCPUSRVSSE[0];
		mpCPUSRV[1]          = mpCPUSRVSSE[1];
		mpShowDepthBufMtrl = mpShowDepthBufMtrlSSE;
		rowPitch			 = 2 * SCREENW * 4;
	}
	else if (mSOCType == AVX_TYPE)
	{
		mpCPURenderTarget[0] = mpCPURenderTargetAVX[0];
		mpCPURenderTarget[1] = mpCPURenderTargetAVX[1];
		mpCPUSRV[0] = mpCPUSRVAVX[0];
		mpCPUSRV[1] = mpCPUSRVAVX[1];
		mpShowDepthBufMtrl = mpShowDepthBufMtrlAVX;
		rowPitch = 2 * SCREENW * 4;
	}
	else if (mSOCType == MASK_AVX_TYPE)
	{
		mpCPURenderTarget[0] = mpCPURenderTargetAVX[0];
		mpCPURenderTarget[1] = mpCPURenderTargetAVX[1];
		mpCPUSRV[0] = mpCPUSRVAVX[0];
		mpCPUSRV[1] = mpCPUSRVAVX[1];
		mpShowDepthBufMtrl = mpShowDepthBufMtrlScalar;
		rowPitch = SCREENW * 4;
	}

    // Call ResizeWindow() because it creates some resources that our blur material needs (e.g., the back buffer)
    ResizeWindow(width, height);

    CPUTRenderStateBlockDX11 *pBlock = new CPUTRenderStateBlockDX11();
    CPUTRenderStateDX11 *pStates = pBlock->GetState();

    // Override default sampler desc for our default shadowing sampler
    pStates->SamplerDesc[1].Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    pStates->SamplerDesc[1].AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].ComparisonFunc = D3D11_COMPARISON_GREATER;
    pBlock->CreateNativeResources();
    CPUTAssetLibrary::GetAssetLibrary()->AddRenderStateBlock( _L("$DefaultRenderStates"), pBlock );
    pBlock->Release(); // We're done with it.  The library owns it now.

    //
    // Load .set files to load the castle scene
	//
	pAssetLibrary->SetMediaDirectoryName(_L("Media\\Castle\\"));

#if 0//#ifdef DEBUG
    mpAssetSetDBR[0] = pAssetLibrary->GetAssetSet(_L("castleLargeOccluders"));
	ASSERT(mpAssetSetDBR[0], _L("Failed loading castle."));

	mpAssetSetDBR[1] = pAssetLibrary->GetAssetSet(_L("groundDebug"));
	ASSERT(mpAssetSetDBR[1], _L("Failed loading ground."));

	mpAssetSetAABB[0] = pAssetLibrary->GetAssetSet(_L("marketStallsDebug"));
	ASSERT(mpAssetSetAABB, _L("Failed loading marketStalls"));

	mpAssetSetAABB[1] = pAssetLibrary->GetAssetSet(_L("castleSmallDecorationsDebug"));
	ASSERT(mpAssetSetAABB, _L("Failed loading castleSmallDecorations"));
#else
    mpAssetSetDBR[0] = pAssetLibrary->GetAssetSet(_L("castleLargeOccluders"));
	ASSERT(mpAssetSetDBR[0], _L("Failed loading castle."));

	mpAssetSetDBR[1] = pAssetLibrary->GetAssetSet(_L("ground"));
	ASSERT(mpAssetSetDBR[1], _L("Failed loading ground."));

	mpAssetSetAABB[0] = pAssetLibrary->GetAssetSet(_L("marketStalls"));
	ASSERT(mpAssetSetAABB, _L("Failed loading marketStalls"));

	mpAssetSetAABB[1] = pAssetLibrary->GetAssetSet(_L("castleSmallDecorations"));
	ASSERT(mpAssetSetAABB, _L("Failed loading castleSmallDecorations"));
#endif

	mpAssetSetSky = pAssetLibrary->GetAssetSet(_L("sky"));
	ASSERT(mpAssetSetSky, _L("Failed loading sky"));

	// For every occluder model in the sene create a place holder 
	// for the CPU transformed vertices of the model.   
	mpDBR->CreateTransformedModels(mpAssetSetDBR, OCCLUDER_SETS);
	// Get number of occluders in the scene
	mNumOccluders = mpDBR->GetNumOccluders();
	// Get number of occluder triangles in the scene 
	mNumOccluderTris = mpDBR->GetNumTriangles();


	// For every occludee model in the scene create a place holder
	// for the triangles that make up the model axis aligned bounding box
	mpAssetSetAABB[2] = mpAssetSetDBR[0];
	mpAssetSetAABB[3] = mpAssetSetDBR[1];

	mpAABB->CreateTransformedAABBoxes(mpAssetSetAABB, OCCLUDEE_SETS);
	// Get number of occludees in the scene
	mNumOccludees = mpAABB->GetNumOccludees();
	// Get number of occluddee triangles in the scene
	mNumOccludeeTris = mpAABB->GetNumTriangles();
	
	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of Models: \t%d"), mNumOccluders);
	mpNumOccludersText->SetText(string);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of tris: \t\t%d"), mNumOccluderTris);
	mpOccluderTrisText->SetText(string);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of Model: \t%d"), mNumOccludees);
	mpNumOccludeesText->SetText(string);

	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tNumber of tris: \t\t%d"), mNumOccludeeTris);
	mpOccludeeTrisText->SetText(string);

	CPUTCheckboxState state;
	if(mEnableCulling)
	{
		state = CPUT_CHECKBOX_CHECKED;
	}
	else 
	{
		state = CPUT_CHECKBOX_UNCHECKED;
	}
	mpCullingCheckBox->SetCheckboxState(state);

	if(mEnableFCulling)
	{
		state = CPUT_CHECKBOX_CHECKED;
	}
	else 
	{
		state = CPUT_CHECKBOX_UNCHECKED;
	}
	mpFCullingCheckBox->SetCheckboxState(state);
	mpDBR->SetEnableFCulling(mEnableFCulling);
	mpAABB->SetEnableFCulling(mEnableFCulling);

	if(mViewDepthBuffer)
	{
		state = CPUT_CHECKBOX_CHECKED;
	}
	else 
	{
		state = CPUT_CHECKBOX_UNCHECKED;
	}
	mpDBCheckBox->SetCheckboxState(state);

	if(mEnableTasks)
	{
		state = CPUT_CHECKBOX_CHECKED;
	}
	else 
	{
		state = CPUT_CHECKBOX_UNCHECKED;
	}
	mpTasksCheckBox->SetCheckboxState(state);

	if(mSyncInterval)
	{
		state = CPUT_CHECKBOX_CHECKED;
	}
	else
	{
		state = CPUT_CHECKBOX_UNCHECKED;
	}
	mpVsyncCheckBox->SetCheckboxState(state);

	if(mPipeline)
	{
		state = CPUT_CHECKBOX_CHECKED;
	}
	else
	{
		state = CPUT_CHECKBOX_UNCHECKED;
	}
	mpPipelineCheckBox->SetCheckboxState(state);

//    if(mOcclusionMatchGPUResolution)
//    {
//        state = CPUT_CHECKBOX_CHECKED;
//    }
//    else
//    {
//        state = CPUT_CHECKBOX_UNCHECKED;
//    }
//    mpOcclusionMatchGPUResolution->SetCheckboxState(state);

	// Setting occluder size threshold in DepthBufferRasterizer
	mpDBR->SetOccluderSizeThreshold(mOccluderSizeThreshold);
	// Setting occludee size threshold in AABBoxRasterizer
	mpAABB->SetOccludeeSizeThreshold(mOccludeeSizeThreshold);
	
	//
	// If no cameras were created from the model sets then create a default simple camera
	// and add it to the camera array.
	//
    if( mpAssetSetDBR[0] && mpAssetSetDBR[0]->GetCameraCount() )
    {
        mpCamera = mpAssetSetDBR[0]->GetFirstCamera();
        mpCamera->AddRef(); 
    } else
    {
        mpCamera = new CPUTCamera();
        CPUTAssetLibraryDX11::GetAssetLibrary()->AddCamera( _L("SampleStart Camera"), mpCamera );

        mpCamera->SetPosition( 0.0f, 0.0f, 5.0f );
        // Set the projection matrix for all of the cameras to match our window.
        // TODO: this should really be a viewport matrix.  Otherwise, all cameras will have the same FOV and aspect ratio, etc instead of just viewport dimensions.
        mpCamera->SetAspectRatio(((float)width)/((float)height));
    }
    mpCamera->SetFov(DirectX::XMConvertToRadians(60.0f)); // TODO: Fix converter's FOV bug (Maya generates cameras for which fbx reports garbage for fov)
    mpCamera->SetFarPlaneDistance(gFarClipDistance);
	mpCamera->SetPosition(27.0f, 2.0f, 47.0f);
	mpCamera->LookAt(41.0f, 8.0f, -50.0f);
    mpCamera->Update();

    // Set up the shadow camera (a camera that sees what the light sees)
    float3 lookAtPoint(0.0f, 0.0f, 0.0f);
    float3 half(1.0f, 1.0f, 1.0f);
    if( mpAssetSetDBR[0] )
    {
        mpAssetSetDBR[0]->GetBoundingBox( &lookAtPoint, &half );
    }
    float length = half.length();

    mpShadowCamera = new CPUTCamera();
    mpShadowCamera->SetFov(DirectX::XMConvertToRadians(45));
    mpShadowCamera->SetAspectRatio(1.0f);
    float fov = mpShadowCamera->GetFov();
    float tanHalfFov = tanf(fov * 0.5f);
    float cameraDistance = length/tanHalfFov;
    float nearDistance = cameraDistance * 0.1f;
    mpShadowCamera->SetNearPlaneDistance(nearDistance);
    mpShadowCamera->SetFarPlaneDistance(2.0f * cameraDistance);
    CPUTAssetLibraryDX11::GetAssetLibrary()->AddCamera( _L("ShadowCamera"), mpShadowCamera );
    float3 shadowCameraPosition = lookAtPoint - gLightDir * cameraDistance;
    mpShadowCamera->SetPosition( shadowCameraPosition );
    mpShadowCamera->LookAt( lookAtPoint.x, lookAtPoint.y, lookAtPoint.z );
    mpShadowCamera->Update();

    mpCameraController = new CPUTCameraControllerFPS();
    mpCameraController->SetCamera(mpCamera);
    mpCameraController->SetLookSpeed(0.004f);
    mpCameraController->SetMoveSpeed(2.5f);

	gLightDir = float3(-40.48f, -142.493f, -3.348f);
	gLightDir = gLightDir.normalize();

	QueryPerformanceFrequency(&glFrequency); 
}

//-----------------------------------------------------------------------------
void MySample::Update(double deltaSeconds)
{
    mpCameraController->Update((float)deltaSeconds);
}

// Handle keyboard events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleKeyboardEvent(CPUTKey key)
{
    static bool panelToggle = false;
    CPUTEventHandledCode    handled = CPUT_EVENT_UNHANDLED;
    cString fileName;
    CPUTGuiControllerDX11*  pGUI = CPUTGetGuiController();

    switch(key)
    {
    case KEY_F1:
        panelToggle = !panelToggle;
        if(panelToggle)
        {
            pGUI->SetActivePanel(ID_SECONDARY_PANEL);
        }
        else
        {
            pGUI->SetActivePanel(ID_MAIN_PANEL);
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    case KEY_L:
        {
            static int cameraObjectIndex = 0;
            CPUTRenderNode *pCameraList[] = { mpCamera, mpShadowCamera };
            cameraObjectIndex = (++cameraObjectIndex) % (sizeof(pCameraList)/sizeof(*pCameraList));
            CPUTRenderNode *pCamera = pCameraList[cameraObjectIndex];
            mpCameraController->SetCamera( pCamera );
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    case KEY_ESCAPE:
        handled = CPUT_EVENT_HANDLED;
        Shutdown();
        break;

	case KEY_1:
		{
			CPUTToggleFullScreenMode();
			break;
		}
	case KEY_2:
		{
			mEnableCulling = !mEnableCulling;
			CPUTCheckboxState state;
			if(mEnableCulling)
			{
				state = CPUT_CHECKBOX_CHECKED;
			}
			else 
			{
				state = CPUT_CHECKBOX_UNCHECKED;
				//memset(mpCPUDepthBuf[mCurrIdx], 0, SCREENW * SCREENH *4);
				memset(mpCPUDepthBuf[mCurrIdx], 0, ((SCREENW*SCREENH * 4 + 31) & 0xFFFFFFE0));

				mpOccludersR2DBText->SetText(         _L("\tDepth rasterized models: 0"));
				mpOccluderRasterizedTrisText->SetText(_L("\tDepth rasterized tris: \t0"));
				mpRasterizeTimeText->SetText(         _L("\tDepth rasterizer time: \t0 ms"));

				mpCulledText->SetText(       _L("\tModels culled: \t\t0"));
				mpVisibleText->SetText(      _L("\tModels visible: \t\t0"));
				mpCulledTrisText->SetText(   _L("\tCulled tris: \t\t0"));
				mpVisibleTrisText->SetText(   _L("\tVisible tris: \t\t0"));
				mpDepthTestTimeText->SetText(_L("\tDepth test time: \t0 ms"));
                mpSOCDepthResolutionText->SetText(_L("\tSOC Depth Resolution: \t0 x 0"));
			}
			mpCullingCheckBox->SetCheckboxState(state);
			break;
		}
	case KEY_3:
		{
			mEnableFCulling = !mEnableFCulling;
			CPUTCheckboxState state;
			if(mEnableFCulling)
			{
				state = CPUT_CHECKBOX_CHECKED;
			}
			else
			{
				state = CPUT_CHECKBOX_UNCHECKED;
				mpDBR->ResetInsideFrustum();
				mpAABB->ResetInsideFrustum();
			}
			mpFCullingCheckBox->SetCheckboxState(state);
			break;			
		}
	case KEY_4:
		{
			mViewDepthBuffer = !mViewDepthBuffer;
			CPUTCheckboxState state;
			if(mViewDepthBuffer)
			{
				state = CPUT_CHECKBOX_CHECKED;
			}
			else 
			{
				state = CPUT_CHECKBOX_UNCHECKED;
			}
			mpDBCheckBox->SetCheckboxState(state);
			break;
		}
	case KEY_5:
		{
			mViewBoundingBox = !mViewBoundingBox;
			CPUTCheckboxState state;
			if(mViewBoundingBox)
			{
				state = CPUT_CHECKBOX_CHECKED;
			}
			else
			{
				state = CPUT_CHECKBOX_UNCHECKED;
			}
			mpBBCheckBox->SetCheckboxState(state);
			break;
		}
	case KEY_6:
		{
			TaskCleanUp();
			mEnableTasks = !mEnableTasks;
			CPUTCheckboxState state;
			
			SAFE_DELETE_ARRAY(mpDBR);
			SAFE_DELETE_ARRAY(mpAABB);

			if(mEnableTasks)
			{
				state = CPUT_CHECKBOX_CHECKED;
				mpPipelineCheckBox->SetVisibility(true);
				mpDepthTestTaskSlider->SetVisibility(true);

				wchar_t string[CPUT_MAX_STRING_LENGTH];
				swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Depth Test Task: \t%d"), mNumDepthTestTasks);
				mpDepthTestTaskSlider->SetText(string);

				SetupOcclusionCullingObjects();
				mpAABB->SetDepthTestTasks(mNumDepthTestTasks);
			}
			else
			{
				state = CPUT_CHECKBOX_UNCHECKED;
				mpPipelineCheckBox->SetVisibility(false);
				mpDepthTestTaskSlider->SetVisibility(false);

				SetupOcclusionCullingObjects();
			}
			mpDBR->CreateTransformedModels(mpAssetSetDBR, OCCLUDER_SETS);		
			mpDBR->SetOccluderSizeThreshold(mOccluderSizeThreshold);
			mpAABB->CreateTransformedAABBoxes(mpAssetSetAABB, OCCLUDEE_SETS);
			mpAABB->SetOccludeeSizeThreshold(mOccludeeSizeThreshold);
			mpTasksCheckBox->SetCheckboxState(state);
			break;
		}
	case KEY_7:
		{
			if(mSyncInterval == 1)
			{
				mSyncInterval = 0;
			}
			else 
			{
				mSyncInterval = 1;
			}
			CPUTCheckboxState state;
			if(mSyncInterval == 1)
			{
				state = CPUT_CHECKBOX_CHECKED;
			}
			else
			{
				state = CPUT_CHECKBOX_UNCHECKED;
			}
			mpVsyncCheckBox->SetCheckboxState(state);
			break;
		}
	case KEY_8:
		{
			TaskCleanUp();
			mPipeline = !mPipeline;
			CPUTCheckboxState state;
			if(mPipeline)
			{
				state = CPUT_CHECKBOX_CHECKED;
			}
			else
			{
				state = CPUT_CHECKBOX_UNCHECKED;
			}
			mpPipelineCheckBox->SetCheckboxState(state);
			break;
		}
//    case KEY_9:
//        {
//            mOcclusionMatchGPUResolution = !mOcclusionMatchGPUResolution;
//            CPUTCheckboxState state;
//            if( mOcclusionMatchGPUResolution )
//            {
//                state = CPUT_CHECKBOX_CHECKED;
//            }
//            else
//            {
//                state = CPUT_CHECKBOX_UNCHECKED;
//            }
//            mpOcclusionMatchGPUResolution->SetCheckboxState( state );
//        } break;
    }
	
    // pass it to the camera controller
    if(handled == CPUT_EVENT_UNHANDLED)
    {
        handled = mpCameraController->HandleKeyboardEvent(key);
    }
    return handled;
}


void MySample::TaskCleanUp()
{
	if(mEnableCulling)
	{
		if(mEnableTasks)
		{
			if(gAABBoxDepthTest[mCurrIdx] != TASKSETHANDLE_INVALID)
			{
				do
				{
					Sleep(10);
				}while(!gTaskMgr.IsSetComplete(gAABBoxDepthTest[mCurrIdx]));
				mpAABB->ReleaseTaskHandles(mCurrIdx);					
			}

			if(gAABBoxDepthTest[mPrevIdx] != TASKSETHANDLE_INVALID)
			{
				do
				{
					Sleep(10);
				}while(!gTaskMgr.IsSetComplete(gAABBoxDepthTest[mPrevIdx]));
				mpAABB->ReleaseTaskHandles(mPrevIdx);			
			}
		}
		mCurrIdx = 0;
		mPrevIdx = 1;
		mFirstFrame = true;
	}
}

// Handle mouse events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state)
{
    if( mpCameraController )
    {
        return mpCameraController->HandleMouseEvent(x, y, wheel, state);
    }
    return CPUT_EVENT_UNHANDLED;
}

static void SplitPath( const std::wstring & inFullPath, std::wstring * outDirectory, std::wstring * outFileName, std::wstring * outFileExt )
{
    wchar_t buffDrive[32];
    wchar_t buffDir[4096];
    wchar_t buffName[4096];
    wchar_t buffExt[4096];

    //assert( !((outDirectory != NULL) && ( (outDirectory != outFileName) || (outDirectory != outFileExt) )) );
    //assert( !((outFileName != NULL) && (outFileName != outFileExt)) );

    _wsplitpath_s( inFullPath.c_str( ), buffDrive, _countof( buffDrive ),
        buffDir, _countof( buffDir ), buffName, _countof( buffName ), buffExt, _countof( buffExt ) );

    if( outDirectory != NULL ) *outDirectory = std::wstring( buffDrive ) + std::wstring( buffDir );
    if( outFileName != NULL )  *outFileName = buffName;
    if( outFileExt != NULL )   *outFileExt = buffExt;
}

std::string SimpleNarrow( const std::wstring & s )
{
    std::string ws;
    ws.resize( s.size( ) );
    for( size_t i = 0; i < s.size( ); i++ ) ws[i] = (char)s[i];
    return ws;
}


std::wstring GetExecutableDirectory( )
{
    wchar_t buffer[4096];

    GetModuleFileName( NULL, buffer, _countof( buffer ) );

    std::wstring outDir;
    SplitPath( buffer, &outDir, NULL, NULL );

    return outDir;
}

bool FileExists( const std::wstring & path )
{
    FILE * fp = NULL;
    _wfopen_s( &fp, path.c_str( ), L"rb" );
    if( fp != NULL )
    {
        fclose( fp );
        return true;
    }
    return false;
}

// Handle any control callback events
//-----------------------------------------------------------------------------
void MySample::HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl )
{
    UNREFERENCED_PARAMETER(Event);
    UNREFERENCED_PARAMETER(pControl);
    cString SelectedItem;
    
    switch(ControlID)
    {
	case ID_FULLSCREEN_BUTTON:
	{
        CPUTToggleFullScreenMode();
        break;
	}
	case ID_RASTERIZE_TYPE:
	{
		TaskCleanUp();
		SAFE_DELETE_ARRAY(mpDBR);
		SAFE_DELETE_ARRAY(mpAABB);
		
		UINT selectedItem;
        mpTypeDropDown->GetSelectedItem(selectedItem);

		mpTasksCheckBox->SetVisibility(true);
		if (mEnableTasks)
		{
			mpPipelineCheckBox->SetVisibility(false);
			mpDepthTestTaskSlider->SetVisibility(false);
		}

		if(selectedItem - 1 == 0)
		{
			mSOCType = SCALAR_TYPE;
			SetupOcclusionCullingObjects();

			mpCPURenderTarget[0] = mpCPURenderTargetScalar[0];
			mpCPURenderTarget[1] = mpCPURenderTargetScalar[1];
			mpCPUSRV[0]          = mpCPUSRVScalar[0];
			mpCPUSRV[1]          = mpCPUSRVScalar[1];
			mpShowDepthBufMtrl   = mpShowDepthBufMtrlScalar;
			rowPitch			 = SCREENW * 4;
		}
		else if(selectedItem - 2 == 0)
		{
			mSOCType = SSE_TYPE;
			SetupOcclusionCullingObjects();

			mpCPURenderTarget[0] = mpCPURenderTargetSSE[0];
			mpCPURenderTarget[1] = mpCPURenderTargetSSE[1];
			mpCPUSRV[0]          = mpCPUSRVSSE[0];
			mpCPUSRV[1]          = mpCPUSRVSSE[1];
			mpShowDepthBufMtrl   = mpShowDepthBufMtrlSSE;
			rowPitch			 = 2 * SCREENW * 4;
		}
		else if (selectedItem - 3 == 0)
		{
			mSOCType = AVX_TYPE;
			SetupOcclusionCullingObjects();

			mpCPURenderTarget[0] = mpCPURenderTargetAVX[0];
			mpCPURenderTarget[1] = mpCPURenderTargetAVX[1];
			mpCPUSRV[0] = mpCPUSRVAVX[0];
			mpCPUSRV[1] = mpCPUSRVAVX[1];
			mpShowDepthBufMtrl = mpShowDepthBufMtrlAVX;
			rowPitch = 2 * SCREENW * 4;
		}
		else if (selectedItem - 4 == 0)
		{
			mSOCType = MASK_AVX_TYPE;
			SetupOcclusionCullingObjects();

			mpCPURenderTarget[0] = mpCPURenderTargetScalar[0];
			mpCPURenderTarget[1] = mpCPURenderTargetScalar[1];
			mpCPUSRV[0] = mpCPUSRVScalar[0];
			mpCPUSRV[1] = mpCPUSRVScalar[1];
			mpShowDepthBufMtrl = mpShowDepthBufMtrlScalar;
			rowPitch = SCREENW * 4;

			// For mask algorithm, disable some of the unused stuff
			// mEnableTasks = false;
            if( mSOCType == MASK_AVX_TYPE )
            {
                mPipeline = false;
                mpPipelineCheckBox->SetCheckboxState(CPUT_CHECKBOX_UNCHECKED);
			    mpPipelineCheckBox->SetVisibility(false);
			    mpDepthTestTaskSlider->SetVisibility(false);
            }
			// mpTasksCheckBox->SetCheckboxState(CPUT_CHECKBOX_UNCHECKED);
			// mpTasksCheckBox->SetVisibility(false);
		}
		mpDBR->CreateTransformedModels(mpAssetSetDBR, OCCLUDER_SETS);		
		mpDBR->SetOccluderSizeThreshold(mOccluderSizeThreshold);
		mpDBR->SetEnableFCulling(mEnableFCulling);
		mpDBR->SetCamera(mpCamera, mCurrIdx);
		mpDBR->ResetInsideFrustum();

		mpAABB->CreateTransformedAABBoxes(mpAssetSetAABB, OCCLUDEE_SETS);
		mpAABB->SetDepthTestTasks(mNumDepthTestTasks);
		mpAABB->SetOccludeeSizeThreshold(mOccludeeSizeThreshold);
		mpAABB->SetEnableFCulling(mEnableFCulling);
		mpAABB->SetCamera(mpCamera, mCurrIdx);
		mpAABB->ResetInsideFrustum();

		break;
	}
	case ID_DEPTH_BUFFER_VISIBLE:
	{
		TaskCleanUp();
		CPUTCheckboxState state = mpDBCheckBox->GetCheckboxState();
		if(state == CPUT_CHECKBOX_CHECKED)
		{
			mViewDepthBuffer = true;
		}
		else 
		{
			mViewDepthBuffer = false;
		}
		break;
	}
	case ID_BOUNDING_BOX_VISIBLE:
	{
		CPUTCheckboxState state = mpBBCheckBox->GetCheckboxState();
		if(state == CPUT_CHECKBOX_CHECKED)
		{
			mViewBoundingBox = true;
		}
		else 
		{
			mViewBoundingBox = false;
		}
		break;
	}
	case ID_ENABLE_TASKS:
	{
		TaskCleanUp();
		SAFE_DELETE_ARRAY(mpDBR);
		SAFE_DELETE_ARRAY(mpAABB);

		CPUTCheckboxState state = mpTasksCheckBox->GetCheckboxState();
		if(state == CPUT_CHECKBOX_CHECKED)
		{
			mEnableTasks = true;
			mpDepthTestTaskSlider->SetVisibility(true);
			mpPipelineCheckBox->SetVisibility(true);
			wchar_t string[CPUT_MAX_STRING_LENGTH];
			swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Depth Test Task: \t%d"), mNumDepthTestTasks);
			mpDepthTestTaskSlider->SetText(string);

			// For mask algorithm, disable some of the unused stuff
			// mEnableTasks = false;
            if( mSOCType == MASK_AVX_TYPE )
            {
                mPipeline = false;
                mpPipelineCheckBox->SetCheckboxState(CPUT_CHECKBOX_UNCHECKED);
			    mpPipelineCheckBox->SetVisibility(false);
			    mpDepthTestTaskSlider->SetVisibility(false);
            }
			
			SetupOcclusionCullingObjects();
			mpAABB->SetDepthTestTasks(mNumDepthTestTasks);
		}
		else
		{
			mEnableTasks = false;
			mpDepthTestTaskSlider->SetVisibility(false);
			mpPipelineCheckBox->SetVisibility(false);
			SetupOcclusionCullingObjects();
		}
		mpDBR->CreateTransformedModels(mpAssetSetDBR, OCCLUDER_SETS);		
		mpDBR->SetOccluderSizeThreshold(mOccluderSizeThreshold);
		mpDBR->SetEnableFCulling(mEnableFCulling);
		mpDBR->SetCamera(mpCamera, mCurrIdx);
		mpDBR->ResetInsideFrustum();

		mpAABB->CreateTransformedAABBoxes(mpAssetSetAABB, OCCLUDEE_SETS);
		mpAABB->SetOccludeeSizeThreshold(mOccludeeSizeThreshold);
		mpAABB->SetEnableFCulling(mEnableFCulling);
		mpAABB->SetCamera(mpCamera, mCurrIdx);
		mpAABB->ResetInsideFrustum();

		break;
	}
	case ID_OCCLUDER_SIZE:
	{
		TaskCleanUp();
		float occluderSize;
		mpOccluderSizeSlider->GetValue(occluderSize);
		mOccluderSizeThreshold = occluderSize;

		wchar_t string[CPUT_MAX_STRING_LENGTH];
		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Occluder Size Threshold: %0.4f"), mOccluderSizeThreshold);
		mpOccluderSizeSlider->SetText(string);

		mpDBR->SetOccluderSizeThreshold(mOccluderSizeThreshold);
		break;
	}
	case ID_OCCLUDEE_SIZE:
	{
		TaskCleanUp();
		float occludeeSize;
		mpOccludeeSizeSlider->GetValue(occludeeSize);
		mOccludeeSizeThreshold = occludeeSize;

		wchar_t string[CPUT_MAX_STRING_LENGTH];
		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Occludee Size Threshold: %0.4f"), mOccludeeSizeThreshold);
		mpOccludeeSizeSlider->SetText(string);
		mpAABB->SetOccludeeSizeThreshold(mOccludeeSizeThreshold);
		break;
	}
	case ID_DEPTH_TEST_TASKS:
	{
		TaskCleanUp();
		float numDepthTestTasks;
		mpDepthTestTaskSlider->GetValue(numDepthTestTasks);
		mNumDepthTestTasks = (UINT)numDepthTestTasks;

		wchar_t string[CPUT_MAX_STRING_LENGTH];
		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Depth Test Task: \t\t%d"), mNumDepthTestTasks);
		mpDepthTestTaskSlider->SetText(string);
		mpAABB->SetDepthTestTasks(mNumDepthTestTasks);
		break;
	}

	case ID_ENABLE_CULLING:
	{
		TaskCleanUp();
		CPUTCheckboxState state = mpCullingCheckBox->GetCheckboxState();
		if(state)
		{
			mEnableCulling = true;
			mFirstFrame = true;
		}
		else
		{
			mEnableCulling = false;
			mFirstFrame = false;
			//memset(mpCPUDepthBuf[mCurrIdx], 0, SCREENW * SCREENH *4);
			memset(mpCPUDepthBuf[mCurrIdx], 0, ((SCREENW*SCREENH * 4 + 31) & 0xFFFFFFE0));

			mpOccludersR2DBText->SetText(         _L("\tDepth rasterized models: 0"));
			mpOccluderRasterizedTrisText->SetText(_L("\tDepth rasterized tris: \t0"));
			mpRasterizeTimeText->SetText(         _L("\tDepth rasterizer time: \t0 ms"));

			mpCulledText->SetText(       _L("\tModels culled: \t\t0"));
			mpVisibleText->SetText(      _L("\tModels visible: \t\t0"));
			mpCulledTrisText->SetText(   _L("\tCulled tris: \t\t0"));
			mpVisibleTrisText->SetText(  _L("\tVisible tris: \t\t0"));
			mpDepthTestTimeText->SetText(_L("\tDepth test time: \t0 ms"));
            mpSOCDepthResolutionText->SetText(_L("\tSOC Depth Resolution: \t0 x 0"));
			mpTotalCullTimeText->SetText(_L("\tTotal Cull time: \t0 ms"));
		}
		break;
	}
	case ID_ENABLE_FCULLING:
	{
		TaskCleanUp();
		CPUTCheckboxState state = mpFCullingCheckBox->GetCheckboxState();
		if(state)
		{
			mEnableFCulling = true;
		}
		else
		{
			mEnableFCulling = false;
			mpDBR->ResetInsideFrustum();
			mpAABB->ResetInsideFrustum();
		}
		mpDBR->SetEnableFCulling(mEnableFCulling);
		mpAABB->SetEnableFCulling(mEnableFCulling);
		break;
	}
	case ID_VSYNC_ON_OFF:
	{
		CPUTCheckboxState state = mpVsyncCheckBox->GetCheckboxState();
		if(state)
		{
			mSyncInterval = 1;
		}
		else
		{
			mSyncInterval = 0;
		}
		break;
	}
	case ID_PIPELINE:
	{
		TaskCleanUp();
		CPUTCheckboxState state = mpPipelineCheckBox->GetCheckboxState();
		if(state)
		{
			mPipeline = true;
			mFirstFrame = true;
		}
		else
		{
			mPipeline = false;
		}
		break;
	}
//    case ID_OCCLUSIONMATCHGPURESOLUTION:
//    {
//        mOcclusionMatchGPUResolution = !mOcclusionMatchGPUResolution;
//        CPUTCheckboxState state;
//        if( mOcclusionMatchGPUResolution )
//        {
//            state = CPUT_CHECKBOX_CHECKED;
//        }
//        else
//        {
//            state = CPUT_CHECKBOX_UNCHECKED;
//        }
//        mpOcclusionMatchGPUResolution->SetCheckboxState( state );
//    } break;
    default:
        break;
    }
}

// Handle resize events
//-----------------------------------------------------------------------------
void MySample::ResizeWindow(UINT width, UINT height)
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    // Before we can resize the swap chain, we must release any references to it.
    // We could have a "AssetLibrary::ReleaseSwapChainResources(), or similar.  But,
    // Generic "release all" works, is simpler to implement/maintain, and is not performance critical.
    pAssetLibrary->ReleaseTexturesAndBuffers();

    // Resize the CPUT-provided render target
    CPUT_DX11::ResizeWindow( width, height );

    // Resize any application-specific render targets here
    if( mpCamera ) mpCamera->SetAspectRatio(((float)width)/((float)height));

    pAssetLibrary->RebindTexturesAndBuffers();
}

static void DepthColorize( const float * inFloatArray, char	* outColorArray )
{
    unsigned char depth;
    float depthfloat;
    int tmpdepth;
    int maxdepth = 0;

    for( int i = 0; i < SCREENW * SCREENH; i++ )
    {
        depthfloat = inFloatArray[i];

        tmpdepth = (int)ceil( depthfloat * 30000 );
        maxdepth = tmpdepth > maxdepth ? tmpdepth : maxdepth;
    }

    float scale = 255.0f / maxdepth;

    for( int i = 0; i < SCREENW * SCREENH; i++ )
    {
        depthfloat = inFloatArray[i];
        if( depthfloat < 0 )
            depthfloat = 0.0f;

        tmpdepth = (int)ceil( depthfloat * 20000 );
        depth = (char)( tmpdepth * scale );
        depth = depth > 255 ? 255 : depth;

        outColorArray[i*4 + 0] = depth;
        outColorArray[i*4 + 1] = depth;
        outColorArray[i*4 + 2] = depth;
        outColorArray[i*4 + 3] = depth;
    }
}

void MySample::UpdateGPUDepthBuf(UINT idx)
{
    DepthColorize( (float*)mpCPUDepthBuf[idx], mpGPUDepthBuf );
}

void MySample::UpdateGPUDepthBuf()
{
	float *pixels = new float[SCREENW*SCREENH];

    if( mEnableTasks )
        gMaskedOcclusionCullingThreadpool->ComputePixelDepthBuffer(pixels, false);
    else
	    gMaskedOcclusionCulling->ComputePixelDepthBuffer(pixels, false);

    DepthColorize( pixels, mpGPUDepthBuf );

	delete[] pixels;
}

//#include "xnamath.h"
//static ID3D11UnorderedAccessView *gpNullUAVs[CPUT_MATERIAL_MAX_TEXTURE_SLOTS] = {0};
//-----------------------------------------------------------------------------
void MySample::Render(double deltaSeconds)
{
    CPUTRenderParametersDX renderParams(mpContext);

#if MOC_RECORDER_ENABLE
    // set this to true here to capture one frame of MOC data
    static bool bTriggerMOCCapture = false;
    
    if( bTriggerMOCCapture && !(mEnableCulling && (mSOCType == MASK_AVX_TYPE)) )
        bTriggerMOCCapture = false;
    if( bTriggerMOCCapture )
    {
        static int captureIndex = 0;

        char fileName[1024]; sprintf_s( fileName, sizeof( fileName ), "OcclusionCulling_%d.mocrec", captureIndex );
        gMaskedOcclusionCulling->StartRecording( fileName );

        captureIndex++;
    }
#endif

	// If mViewBoundingBox is enabled then draw the axis aligned bounding box 
	// for all the model in the scene. FYI This will affect frame rate.
	if(mViewBoundingBox)
	{
		renderParams.mShowBoundingBoxes = true;
	}
	else
	{
		renderParams.mShowBoundingBoxes = false;
	}	

	if(mEnableCulling) 
	{
		if(!mFirstFrame)
		{
			UINT tmpId = mCurrIdx;
			mCurrIdx = mPrevIdx;
			mPrevIdx = tmpId;
		}	
	}

	mCameraCopy[mCurrIdx] = *mpCamera;

	// Clear back buffer
	const float clearColor[] = { 0.0993f, 0.0993f, 0.0993f, 1.0f };
    mpContext->ClearRenderTargetView( mpBackBufferRTV,  clearColor );
    mpContext->ClearDepthStencilView( mpDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

	// Set the camera transforms so that the occluders can be transformed 
	mpDBR->SetViewProj(mCameraCopy[mCurrIdx].GetViewMatrix(), (float4x4*)mCameraCopy[mCurrIdx].GetProjectionMatrix(), mCurrIdx);

	// Set the camera transforms so that the occludee abix aligned bounding boxes (AABB) can be transformed
	mpAABB->SetViewProjMatrix(mCameraCopy[mCurrIdx].GetViewMatrix(), (float4x4*)mCameraCopy[mCurrIdx].GetProjectionMatrix(), mCurrIdx);

	// If view frustum culling is enabled then determine which occluders and occludees are 
	// inside the view frustum and run the software occlusion culling on only the those models
	if(mEnableFCulling)
	{
		renderParams.mRenderOnlyVisibleModels = true;
	}
	else
	{
		renderParams.mRenderOnlyVisibleModels = false;
	}

	// if software occlusion culling is enabled
	if(mEnableCulling)
	{
        LARGE_INTEGER totalCullTimeStartTime;
        QueryPerformanceCounter( &totalCullTimeStartTime );

        if( mSOCType == MASK_AVX_TYPE && mEnableTasks )
        {
            // now done in the culling code itself just before starting work
            //gMaskedOcclusionCullingThreadpool->WakeThreads();
        }

		// Set the Depth Buffer
		mpCPURenderTargetPixels = (UINT*)mpCPUDepthBuf[mCurrIdx];
		
		mpDBR->SetCPURenderTargetPixels(mpCPURenderTargetPixels, mCurrIdx);
		// Transform the occluder models and rasterize them to the depth buffer
		mpDBR->TransformModelsAndRasterizeToDepthBuffer(&mCameraCopy[mCurrIdx], mCurrIdx);
	
		
		mpAABB->SetCPURenderTargetPixels(mpCPURenderTargetPixels, mCurrIdx);
		mpAABB->SetDepthSummaryBuffer(mpDBR->GetDepthSummaryBuffer(mCurrIdx), mCurrIdx);
		// Transform the occludee AABB, rasterize and depth test to determine is occludee is visible or occluded 
		mpAABB->TransformAABBoxAndDepthTest(&mCameraCopy[mCurrIdx], mCurrIdx);		

		if(mEnableTasks)
		{
			if(mPipeline)
			{
				if(mFirstFrame)
				{
					mFirstFrame = false;
				}
				else
				{
					if(!gTaskMgr.IsSetComplete(gAABBoxDepthTest[mPrevIdx]))
					{
						mpAABB->WaitForTaskToFinish(mPrevIdx);
					}
					mpAABB->ReleaseTaskHandles(mPrevIdx);
				}
			}
			else
			{
				mpAABB->WaitForTaskToFinish(mCurrIdx);
				mpAABB->ReleaseTaskHandles(mCurrIdx);
			}
		}

        LARGE_INTEGER totalCullTimeEndTime;
        QueryPerformanceCounter( &totalCullTimeEndTime );

        mTotalCullTimeLastIndex = ( mTotalCullTimeLastIndex + 1 ) % _countof( mTotalCullTimeHistories );
        mTotalCullTimeHistories[mTotalCullTimeLastIndex]= ( (double)( totalCullTimeEndTime.QuadPart - totalCullTimeStartTime.QuadPart ) ) / ( (double)glFrequency.QuadPart );
        mTotalCullTimeAvg = 0.0;
        for ( int i = 0; i < _countof( mTotalCullTimeHistories ); i++ )
        {
            mTotalCullTimeAvg += mTotalCullTimeHistories[i];
        }
        mTotalCullTimeAvg /= (double)_countof( mTotalCullTimeHistories );
    }
	
	// If mViewDepthBuffer is enabled then blit the CPU rasterized depth buffer to the frame buffer
	if(mViewDepthBuffer)
	{
		mpShowDepthBufMtrl->SetRenderStates(renderParams);
		if (mSOCType == MASK_AVX_TYPE)
		{
			UpdateGPUDepthBuf();
			mpContext->UpdateSubresource(mpCPURenderTarget[mCurrIdx], 0, NULL, mpGPUDepthBuf, rowPitch, 0);
		}
		else
		{
			if (mEnableCulling && mEnableTasks && mPipeline)
			{
				// Update the GPU-side depth buffer
				UpdateGPUDepthBuf(mPrevIdx);
				mpContext->UpdateSubresource(mpCPURenderTarget[mPrevIdx], 0, NULL, mpGPUDepthBuf, rowPitch, 0);
			}
			else
			{
				// Update the GPU-side depth buffer
				UpdateGPUDepthBuf(mCurrIdx);
				mpContext->UpdateSubresource(mpCPURenderTarget[mCurrIdx], 0, NULL, mpGPUDepthBuf, rowPitch, 0);
			}
		}
    }

    if( mEnableCulling && (mSOCType == MASK_AVX_TYPE) && mEnableTasks )
    {
        // clear for the next frame
        mpDBMRAVXMT->MOCDepthSetDirty();
    }

	// If mViewDepthBuffer, debug draw CPU rasterized depth
	if(mViewDepthBuffer)
	{
		mpContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		mpContext->Draw(3, 0);
    }
	// else render the (frustum culled) occluders and only the visible occludees
	else
	{
		CPUTMeshDX11::ResetDrawCallCount();

		if(mpAssetSetAABB) 
		{
			// if occlusion culling is enabled then render only the visible occludees in the scene
			if(mEnableCulling)
			{
				if(mEnableTasks && mPipeline)
				{
					renderParams.mpCamera = &mCameraCopy[mPrevIdx];
					mpAABB->RenderVisible(mpAssetSetAABB, renderParams, OCCLUDEE_SETS, mPrevIdx); 
				}
				else
				{
					renderParams.mpCamera = &mCameraCopy[mCurrIdx];
					mpAABB->RenderVisible(mpAssetSetAABB, renderParams, OCCLUDEE_SETS, mCurrIdx); 
				}
			}
			// else render all the (25,000) occludee models in the scene
			else
			{
				// render shadow map begin
				renderParams.mpCamera = mpShadowCamera;
				//mpAABB->RenderShadow

				// render shadow map end

				renderParams.mpCamera = &mCameraCopy[mCurrIdx];
				mpAABB->Render(mpAssetSetAABB, renderParams, OCCLUDEE_SETS, mCurrIdx);
			}
			if(mpAssetSetSky) { mpAssetSetSky->RenderRecursive(renderParams); }
		}
		mNumDrawCalls = CPUTMeshDX11::GetDrawCallCount();
	}

	wchar_t string[CPUT_MAX_STRING_LENGTH];
	if(mEnableCulling)
	{
		if(mEnableTasks && mPipeline)
		{
     		mpDBR->ComputeR2DBTime(mPrevIdx);
			mNumOccludersR2DB = mpDBR->GetNumOccludersR2DB(mPrevIdx);
			mNumOccluderRasterizedTris = mpDBR->GetNumRasterizedTriangles(mPrevIdx);
			mNumCulled = mpAABB->GetNumCulled(mPrevIdx);
			mNumOccludeeCulledTris = mpAABB->GetNumCulledTriangles(mPrevIdx);
		}
		else
		{
			mpDBR->ComputeR2DBTime(mCurrIdx);
			mNumOccludersR2DB = mpDBR->GetNumOccludersR2DB(mCurrIdx);
			mNumOccluderRasterizedTris = mpDBR->GetNumRasterizedTriangles(mCurrIdx);
			mNumCulled = mpAABB->GetNumCulled(mCurrIdx);
			mNumOccludeeCulledTris = mpAABB->GetNumCulledTriangles(mCurrIdx);
		}
		mRasterizeTime = mpDBR->GetRasterizeTime();
		
        if( mSOCType == MASK_AVX_TYPE && mEnableTasks )
        {
            // now in AABBoxMaskedRasterizerAVXMT::TransformAABBoxAndDepthTest
            //gMaskedOcclusionCullingThreadpool->SuspendThreads( );
        }

		mNumVisible = mNumOccludees - mNumCulled;
		mNumOccludeeVisibleTris = mNumOccludeeTris - mNumOccludeeCulledTris;
		
		mDepthTestTime = mpAABB->GetDepthTestTime();
		mTotalCullTime = mRasterizeTime + mDepthTestTime;
		
		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tDepth rasterized models: %d"), mNumOccludersR2DB);
		mpOccludersR2DBText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tDepth rasterized tris: \t%u"), (unsigned int)mNumOccluderRasterizedTris);
		mpOccluderRasterizedTrisText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tDepth rasterizer time: \t%0.2f ms"), mRasterizeTime * 1000.0f);
		mpRasterizeTimeText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tModels culled: \t\t%d"), mNumCulled);
		mpCulledText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tModels visible: \t\t%d"), mNumVisible);
		mpVisibleText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tCulled tris: \t\t%u"), (unsigned int)mNumOccludeeCulledTris);
		mpCulledTrisText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tVisible tris: \t\t%u"), (unsigned int)mNumOccludeeVisibleTris);
		mpVisibleTrisText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tDepth test time: \t%0.2f ms"), mDepthTestTime * 1000.0f);
		mpDepthTestTimeText->SetText(string);

        swprintf_s( &string[0], CPUT_MAX_STRING_LENGTH, _L( "\tSOC Depth Resolution: \t%d x %d" ), SCREENW, SCREENH );
        mpSOCDepthResolutionText->SetText(string);

		//swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tTotal Cull time: \t%0.2f ms"), mTotalCullTime * 1000.0f);
        swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tTotal Cull time: \t%0.2f ms"), mTotalCullTimeAvg * 1000.0f);
		mpTotalCullTimeText->SetText(string);
	}
	else
	{
		UINT fCullCount = 0;
		if(mEnableFCulling)
		{
			fCullCount = mpAABB->GetNumFCullCount();
		}

		mNumCulled = mpAABB->GetNumCulled(mCurrIdx) + fCullCount;
		mNumOccludeeVisibleTris = mpAABB->GetNumTrisRendered();

		mNumOccludeeCulledTris = mNumOccludeeTris - mNumOccludeeVisibleTris;
		mNumVisible = mNumOccludees - mNumCulled;
		
		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tModels culled: \t\t%d"), mNumCulled);
		mpCulledText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tModels visible: \t\t%d"), mNumVisible);
		mpVisibleText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tCulled tris: \t\t%d"), (int)mNumOccludeeCulledTris);
		mpCulledTrisText->SetText(string);

		swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("\tVisible tris: \t\t%d"), (int)mNumOccludeeVisibleTris);
		mpVisibleTrisText->SetText(string);
	}
	
	swprintf_s(&string[0], CPUT_MAX_STRING_LENGTH, _L("Number of draw calls: \t\t %d"), mNumDrawCalls);
	mpDrawCallsText->SetText(string);
	
    {
        CPUTDrawGUI();
    }

#if MOC_RECORDER_ENABLE
    if( bTriggerMOCCapture )
    {
        gMaskedOcclusionCulling->StopRecording();
        bTriggerMOCCapture = false;
    }
#endif
}



void MySample::RunCpuid(uint32_t eax, uint32_t ecx, uint32_t* abcd)
{
	__cpuidex(reinterpret_cast<int32_t*>(abcd), eax, ecx);
}

int MySample::CheckXcr0Ymm()
{
	uint32_t xcr0;
	xcr0 = (uint32_t)_xgetbv(0);
	return ((xcr0 & 6) == 6); /* checking if xmm and ymm state are enabled in XCR0 */
}

int MySample::Check4thGenIntelCoreFeatures()
{
	uint32_t abcd[4];
	uint32_t fma_movbe_osxsave_mask = ((1 << 12) | (1 << 22) | (1 << 27));
	uint32_t avx2_bmi12_mask = (1 << 5) | (1 << 3) | (1 << 8);
	/* CPUID.(EAX=01H, ECX=0H):ECX.FMA[bit 12]==1 &&
	CPUID.(EAX=01H, ECX=0H):ECX.MOVBE[bit 22]==1 &&
	CPUID.(EAX=01H, ECX=0H):ECX.OSXSAVE[bit 27]==1 */
	RunCpuid(1, 0, abcd);
	if ((abcd[2] & fma_movbe_osxsave_mask) != fma_movbe_osxsave_mask)
		return 0;
	if (!CheckXcr0Ymm())
		return 0;
	/* CPUID.(EAX=07H, ECX=0H):EBX.AVX2[bit 5]==1 &&
	CPUID.(EAX=07H, ECX=0H):EBX.BMI1[bit 3]==1 &&
	CPUID.(EAX=07H, ECX=0H):EBX.BMI2[bit 8]==1 */
	RunCpuid(7, 0, abcd);
	if ((abcd[1] & avx2_bmi12_mask) != avx2_bmi12_mask)
		return 0;
	/* CPUID.(EAX=80000001H):ECX.LZCNT[bit 5]==1 */
	RunCpuid(0x80000001, 0, abcd);
	if ((abcd[2] & (1 << 5)) == 0)
		return 0;
	return 1;
}

int MySample::CanUseIntelCore4thGenFeatures()
{
	int the_4th_gen_features_available = -1;
	if (the_4th_gen_features_available < 0)
		the_4th_gen_features_available = Check4thGenIntelCoreFeatures();
	return the_4th_gen_features_available;
}

char *gpDefaultShaderSource =  "\n\
// ********************************************************************************************************\n\
struct VS_INPUT\n\
{\n\
    float3 Pos      : POSITION; // Projected position\n\
    float3 Norm     : NORMAL;\n\
    float2 Uv       : TEXCOORD0;\n\
};\n\
struct PS_INPUT\n\
{\n\
    float4 Pos      : SV_POSITION;\n\
    float3 Norm     : NORMAL;\n\
    float2 Uv       : TEXCOORD0;\n\
    float4 LightUv  : TEXCOORD1;\n\
    float3 Position : TEXCOORD2; // Object space position \n\
};\n\
// ********************************************************************************************************\n\
    Texture2D    TEXTURE0 : register( t0 );\n\
    SamplerState SAMPLER0 : register( s0 );\n\
// ********************************************************************************************************\n\
cbuffer cbPerModelValues\n\
{\n\
    row_major float4x4 World : WORLD;\n\
    row_major float4x4 WorldViewProjection : WORLDVIEWPROJECTION;\n\
    row_major float4x4 InverseWorld : INVERSEWORLD;\n\
              float4   LightDirection;\n\
              float4   EyePosition;\n\
    row_major float4x4 LightWorldViewProjection;\n\
};\n\
// ********************************************************************************************************\n\
// TODO: Note: nothing sets these values yet\n\
cbuffer cbPerFrameValues\n\
{\n\
    row_major float4x4  View;\n\
    row_major float4x4  Projection;\n\
};\n\
// ********************************************************************************************************\n\
PS_INPUT VSMain( VS_INPUT input )\n\
{\n\
    PS_INPUT output = (PS_INPUT)0;\n\
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );\n\
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;\n\
    // TODO: transform the light into object space instead of the normal into world space\n\
    output.Norm = mul( input.Norm, (float3x3)World );\n\
    output.Uv   = float2(input.Uv.x, input.Uv.y);\n\
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );\n\
    return output;\n\
}\n\
// ********************************************************************************************************\n\
float4 PSMain( PS_INPUT input ) : SV_Target\n\
{\n\
    float3  lightUv = input.LightUv.xyz / input.LightUv.w;\n\
    lightUv.xy = lightUv.xy * 0.5f + 0.5f; // TODO: Move scale and offset to matrix.\n\
    lightUv.y  = 1.0f - lightUv.y;\n\
    float3 normal         = normalize(input.Norm);\n\
    float  nDotL          = saturate( dot( normal, -LightDirection ) );\n\
    float3 eyeDirection   = normalize(input.Position - EyePosition);\n\
    float3 reflection     = reflect( eyeDirection, normal );\n\
    float  rDotL          = saturate(dot( reflection, -LightDirection ));\n\
    float3 specular       = pow(rDotL, 16.0f);\n\
    float4 diffuseTexture = TEXTURE0.Sample( SAMPLER0, input.Uv );\n\
    float ambient = 0.05;\n\
    float3 result = (nDotL+ambient) * diffuseTexture + specular;\n\
    return float4( result, 1.0f );\n\
}\n\
\n\
// ********************************************************************************************************\n\
struct VS_INPUT_NO_TEX\n\
{\n\
    float3 Pos      : POSITION; // Projected position\n\
    float3 Norm     : NORMAL;\n\
};\n\
struct PS_INPUT_NO_TEX\n\
{\n\
    float4 Pos      : SV_POSITION;\n\
    float3 Norm     : NORMAL;\n\
    float4 LightUv  : TEXCOORD1;\n\
    float3 Position : TEXCOORD0; // Object space position \n\
};\n\
// ********************************************************************************************************\n\
PS_INPUT_NO_TEX VSMainNoTexture( VS_INPUT_NO_TEX input )\n\
{\n\
    PS_INPUT_NO_TEX output = (PS_INPUT_NO_TEX)0;\n\
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );\n\
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;\n\
    // TODO: transform the light into object space instead of the normal into world space\n\
    output.Norm = mul( input.Norm, (float3x3)World );\n\
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );\n\
    return output;\n\
}\n\
// ********************************************************************************************************\n\
float4 PSMainNoTexture( PS_INPUT_NO_TEX input ) : SV_Target\n\
{\n\
    float3 lightUv = input.LightUv.xyz / input.LightUv.w;\n\
    float2 uv = lightUv.xy * 0.5f + 0.5f;\n\
    float3 eyeDirection = normalize(input.Position - EyePosition.xyz);\n\
    float3 normal       = normalize(input.Norm);\n\
    float  nDotL = saturate( dot( normal, -normalize(LightDirection.xyz) ) );\n\
    float3 reflection   = reflect( eyeDirection, normal );\n\
    float  rDotL        = saturate(dot( reflection, -LightDirection.xyz ));\n\
    float  specular     = 0.2f * pow( rDotL, 4.0f );\n\
    return float4( (nDotL + specular).xxx, 1.0f);\n\
}\n\
";

typedef BOOL( WINAPI *LPFN_GLPI )(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
    PDWORD );


// Helper function to count set bits in the processor mask.
static DWORD CountSetBits( ULONG_PTR bitMask )
{
    DWORD LSHIFT = sizeof( ULONG_PTR ) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for( i = 0; i <= LSHIFT; ++i )
    {
        bitSetCount += ( ( bitMask & bitTest ) ? 1 : 0 );
        bitTest /= 2;
    }

    return bitSetCount;
}

static void GetCPUCoreCountInfo( int & physicalPackages, int & physicalCores, int & logicalCores )
{
    // how difficult could it be to get logical core count?
    // well, apparently, according to https://msdn.microsoft.com/en-us/library/windows/desktop/ms683194%28v=vs.85%29.aspx,  difficult:

    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;

    physicalPackages = -1;
    physicalCores = -1;
    logicalCores = -1;

    glpi = (LPFN_GLPI)GetProcAddress(
        GetModuleHandle( TEXT( "kernel32" ) ),
        "GetLogicalProcessorInformation" );
    if( NULL == glpi )
    {
        _tprintf( TEXT( "\nGetLogicalProcessorInformation is not supported.\n" ) );
        assert( false );
        return;
    }

    while( !done )
    {
        DWORD rc = glpi( buffer, &returnLength );

        if( FALSE == rc )
        {
            if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
            {
                if( buffer )
                    free( buffer );

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                    returnLength );

                if( NULL == buffer )
                {
                    _tprintf( TEXT( "\nError: Allocation failure\n" ) );
                    assert( false );
                    return;
                }
            }
            else
            {
                _tprintf( TEXT( "\nError %d\n" ), GetLastError( ) );
                assert( false );
                return;
            }
        }
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while( byteOffset + sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION ) <= returnLength )
    {
        switch( ptr->Relationship )
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            numaNodeCount++;
            break;

        case RelationProcessorCore:
            processorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += CountSetBits( ptr->ProcessorMask );
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if( Cache->Level == 1 )
            {
                processorL1CacheCount++;
            }
            else if( Cache->Level == 2 )
            {
                processorL2CacheCount++;
            }
            else if( Cache->Level == 3 )
            {
                processorL3CacheCount++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            _tprintf( TEXT( "\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n" ) );
            break;
        }
        byteOffset += sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION );
        ptr++;
    }

    //_tprintf( TEXT( "\nGetLogicalProcessorInformation results:\n" ) );
    //_tprintf( TEXT( "Number of NUMA nodes: %d\n" ),
    //    numaNodeCount );
    //_tprintf( TEXT( "Number of physical processor packages: %d\n" ),
    //    processorPackageCount );
    //_tprintf( TEXT( "Number of processor cores: %d\n" ),
    //    processorCoreCount );
    //_tprintf( TEXT( "Number of logical processors: %d\n" ),
    //    logicalProcessorCount );
    //_tprintf( TEXT( "Number of processor L1/L2/L3 caches: %d/%d/%d\n" ),
    //    processorL1CacheCount,
    //    processorL2CacheCount,
    //    processorL3CacheCount );
    //
    //
    free( buffer );

    physicalPackages = processorPackageCount;
    physicalCores = processorCoreCount;
    logicalCores = logicalProcessorCount;
}

static unsigned int GetOptimalNumberOfThreads( )
{
    int physicalPackages;
    int physicalCores;
    int logicalCores;

    GetCPUCoreCountInfo( physicalPackages, physicalCores, logicalCores );

    return max( physicalCores, logicalCores - 1 );
}
