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
#ifndef __CPUT_SAMPLESTARTDX11_H__
#define __CPUT_SAMPLESTARTDX11_H__

#include <DirectxMath.h>
#include <stdio.h>
#include "CPUT_DX11.h"
#include <D3D11.h>
#include <time.h>
#include "CPUTSprite.h"

#include "DepthBufferRasterizerScalarST.h"
#include "DepthBufferRasterizerScalarMT.h"

#include "DepthBufferRasterizerSSEST.h"
#include "DepthBufferRasterizerSSEMT.h"


#include "DepthBufferRasterizerAVXST.h"
#include "DepthBufferRasterizerAVXMT.h"

#include "AABBoxRasterizerScalarST.h"
#include "AABBoxRasterizerScalarMT.h"

#include "AABBoxRasterizerSSEST.h"
#include "AABBoxRasterizerSSEMT.h"

#include "DepthBufferMaskedRasterizerAVXST.h"
#include "AABBoxMaskedRasterizerAVXST.h"

#include "DepthBufferMaskedRasterizerAVXMT.h"
#include "AABBoxMaskedRasterizerAVXMT.h"

#include "AABBoxRasterizerAVXST.h"
#include "AABBoxRasterizerAVXMT.h"

#include "TaskMgrTBB.h"



//-----------------------------------------------------------------------------
class MySample : public CPUT_DX11
{
private:
    float                  mfElapsedTime;
   
    CPUTCameraController  *mpCameraController;
    CPUTSprite            *mpDebugSprite;

    CPUTAssetSet          *mpShadowCameraSet;
    CPUTRenderTargetDepth *mpShadowRenderTarget;

    CPUTText              *mpFPSCounter;
	CPUTDropdown		  *mpTypeDropDown;
	CPUTCamera			   mCameraCopy[2];

	CPUTText			  *mpOccludersText;
	CPUTText			  *mpNumOccludersText;
	CPUTText			  *mpOccludersR2DBText;
	CPUTText			  *mpOccluderTrisText;
	CPUTText			  *mpOccluderRasterizedTrisText;
	CPUTText		      *mpRasterizeTimeText;
	CPUTSlider			  *mpOccluderSizeSlider;

	CPUTText			  *mpOccludeesText;
	CPUTText			  *mpNumOccludeesText;
	CPUTText			  *mpCulledText;
	CPUTText			  *mpVisibleText;
	CPUTText			  *mpOccludeeTrisText;
	CPUTText			  *mpCulledTrisText;
	CPUTText			  *mpVisibleTrisText;
	CPUTText			  *mpDepthTestTimeText;
    CPUTText			  *mpSOCDepthResolutionText;
    CPUTSlider			  *mpOccludeeSizeSlider;

	CPUTText			  *mpTotalCullTimeText;

	CPUTCheckbox		  *mpCullingCheckBox;
	CPUTCheckbox		  *mpFCullingCheckBox;
	CPUTCheckbox		  *mpDBCheckBox;
	CPUTCheckbox		  *mpBBCheckBox;
	CPUTCheckbox		  *mpTasksCheckBox;
	CPUTCheckbox		  *mpVsyncCheckBox;
	CPUTCheckbox		  *mpPipelineCheckBox;
    //CPUTCheckbox		  *mpOcclusionMatchGPUResolution;

	CPUTText		      *mpDrawCallsText;
	CPUTSlider			  *mpDepthTestTaskSlider;

	CPUTAssetSet	      *mpAssetSetDBR[OCCLUDER_SETS];
	CPUTAssetSet		  *mpAssetSetAABB[OCCLUDEE_SETS];
	CPUTAssetSet		  *mpAssetSetSky;

	CPUTMaterialDX11		 *mpShowDepthBufMtrlScalar;
	CPUTMaterialDX11		 *mpShowDepthBufMtrlSSE;
	CPUTMaterialDX11		 *mpShowDepthBufMtrlAVX;
	CPUTMaterialDX11		 *mpShowDepthBufMtrl;
	
	char					 *mpCPUDepthBuf[2];
	char					 *mpGPUDepthBuf;
	
	ID3D11Texture2D          *mpCPURenderTargetScalar[2];
	ID3D11Texture2D          *mpCPURenderTargetSSE[2];
	ID3D11Texture2D          *mpCPURenderTargetAVX[2];
	ID3D11Texture2D          *mpCPURenderTarget[2];

	ID3D11ShaderResourceView *mpCPUSRVScalar[2];
	ID3D11ShaderResourceView *mpCPUSRVSSE[2];
	ID3D11ShaderResourceView *mpCPUSRVAVX[2];
	ID3D11ShaderResourceView *mpCPUSRV[2];

	UINT					 rowPitch;

	SOC_TYPE mSOCType;
	DepthBufferRasterizer	 		*mpDBR;
	DepthBufferRasterizerScalarST	*mpDBRScalarST;
	DepthBufferRasterizerScalarMT	*mpDBRScalarMT;
	DepthBufferRasterizerSSEST		*mpDBRSSEST;
	DepthBufferRasterizerSSEMT		*mpDBRSSEMT;
	DepthBufferRasterizerAVXST		*mpDBRAVXST;
	DepthBufferRasterizerAVXMT		*mpDBRAVXMT;
	DepthBufferMaskedRasterizerAVXST *mpDBMRAVXST;
    DepthBufferMaskedRasterizerAVXMT *mpDBMRAVXMT;

	AABBoxRasterizer				*mpAABB;
	AABBoxRasterizerScalarST		*mpAABBScalarST;
	AABBoxRasterizerScalarMT		*mpAABBScalarMT;
	AABBoxRasterizerSSEST			*mpAABBSSEST;
	AABBoxRasterizerSSEMT			*mpAABBSSEMT;
	AABBoxRasterizerAVXST			*mpAABBAVXST;
	AABBoxRasterizerAVXMT			*mpAABBAVXMT;
	AABBoxMaskedRasterizerAVXST		*mpAABBMAVXST;
    AABBoxMaskedRasterizerAVXMT		*mpAABBMAVXMT;

	UINT				mNumOccluders;
	UINT				mNumOccludersR2DB;
	UINT				mNumOccluderTris;
	UINT    			mNumOccluderRasterizedTris;
	double				mRasterizeTime;
	float				mOccluderSizeThreshold;
	
	UINT				mNumOccludees;
	UINT				mNumCulled;
	UINT				mNumVisible;
	UINT	   			mNumOccludeeTris;
	UINT    			mNumOccludeeCulledTris;
	UINT    			mNumOccludeeVisibleTris;
	double				mDepthTestTime;
	float				mOccludeeSizeThreshold;

	double				mTotalCullTime;

	bool				mEnableCulling;
	bool				mEnableFCulling;
	bool				mViewDepthBuffer;
	bool				mViewBoundingBox;
	bool				mEnableTasks;
	bool				mPipeline;

	UINT				mNumDrawCalls;
	UINT				mNumDepthTestTasks;
	UINT				mCurrIdx;
	UINT				mPrevIdx;
	bool				mFirstFrame;

    //bool                mOcclusionMatchGPUResolution;

    double              mTotalCullTimeHistories[300];
    int                 mTotalCullTimeLastIndex;
    double              mTotalCullTimeAvg;


	void				SetupOcclusionCullingObjects();

public:
	MySample();
	virtual ~MySample();

	UINT			*mpCPURenderTargetPixels;

    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);
    virtual void                 HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl );

    virtual void Create();
    virtual void Render(double deltaSeconds);
	virtual void RunCpuid(uint32_t eax, uint32_t ecx, uint32_t * abcd);
	virtual int  CheckXcr0Ymm();
	virtual int  Check4thGenIntelCoreFeatures();
	virtual int  CanUseIntelCore4thGenFeatures();
    virtual void Update(double deltaSeconds);
    virtual void ResizeWindow(UINT width, UINT height);
	virtual void TaskCleanUp();
	virtual void UpdateGPUDepthBuf();
	virtual void UpdateGPUDepthBuf(UINT idx);

	// define some controls1
	static const CPUTControlID ID_MAIN_PANEL = 10;
	static const CPUTControlID ID_SECONDARY_PANEL = 20;
	static const CPUTControlID ID_FULLSCREEN_BUTTON = 100;
	static const CPUTControlID ID_NEXTMODEL_BUTTON = 200;
	static const CPUTControlID ID_TEST_CONTROL = 1000;
	static const CPUTControlID ID_IGNORE_CONTROL_ID = -1;

	static const CPUTControlID ID_RASTERIZE_TYPE = 1100;

	static const CPUTControlID ID_OCCLUDERS = 1200;
	static const CPUTControlID ID_NUM_OCCLUDERS = 1300;
	static const CPUTControlID ID_NUM_OCCLUDERS_RASTERIZED_TO_DB = 1400;
	static const CPUTControlID ID_NUM_OCCLUDER_TRIS = 1500;
	static const CPUTControlID ID_NUM_OCCLUDER_RASTERIZED_TRIS = 1600;
	static const CPUTControlID ID_RASTERIZE_TIME = 1700;
	static const CPUTControlID ID_OCCLUDER_SIZE = 1800;

	static const CPUTControlID ID_OCCLUDEES = 1900;
	static const CPUTControlID ID_NUM_OCCLUDEES = 2000;
	static const CPUTControlID ID_NUM_CULLED = 2100;
	static const CPUTControlID ID_NUM_VISIBLE = 2150;
	static const CPUTControlID ID_NUM_OCCLUDEE_TRIS = 2200;
	static const CPUTControlID ID_NUM_OCCLUDEE_CULLED_TRIS = 2300;
	static const CPUTControlID ID_NUM_OCCLUDEE_VISIBLE_TRIS = 2350;
	static const CPUTControlID ID_DEPTHTEST_TIME = 2400;
    static const CPUTControlID ID_SOCDEPTHRESOLUTIONTEXT = 2400;
	static const CPUTControlID ID_OCCLUDEE_SIZE = 2500;

	static const CPUTControlID ID_TOTAL_CULL_TIME = 2600;

	static const CPUTControlID ID_ENABLE_CULLING = 2700;
	static const CPUTControlID ID_ENABLE_FCULLING = 2800;
	static const CPUTControlID ID_DEPTH_BUFFER_VISIBLE = 2900;
	static const CPUTControlID ID_BOUNDING_BOX_VISIBLE = 3000;
	static const CPUTControlID ID_ENABLE_TASKS = 3100;
	static const CPUTControlID ID_NUM_DRAW_CALLS = 3200;
	static const CPUTControlID ID_DEPTH_TEST_TASKS = 3300;
	static const CPUTControlID ID_VSYNC_ON_OFF = 3400;
	static const CPUTControlID ID_PIPELINE = 3500;
    static const CPUTControlID ID_OCCLUSIONMATCHGPURESOLUTION = 3600;
};
#endif // __CPUT_SAMPLESTARTDX11_H__
