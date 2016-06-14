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
//--------------------------------------------------------------------------------------
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

#include "DepthBufferMaskedRasterizerAVXST.h"

#include "DepthBufferRasterizerAVXST.h"
#include "DepthBufferRasterizerAVXMT.h"

#include "AABBoxRasterizerScalarST.h"
#include "AABBoxRasterizerScalarMT.h"

#include "AABBoxRasterizerSSEST.h"
#include "AABBoxRasterizerSSEMT.h"

#include "AABBoxMaskedRasterizerAVXST.h"

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
	CPUTSlider			  *mpOccludeeSizeSlider;

	CPUTText			  *mpTotalCullTimeText;

	CPUTCheckbox		  *mpCullingCheckBox;
	CPUTCheckbox		  *mpFCullingCheckBox;
	CPUTCheckbox		  *mpDBCheckBox;
	CPUTCheckbox		  *mpBBCheckBox;
	CPUTCheckbox		  *mpTasksCheckBox;
	CPUTCheckbox		  *mpVsyncCheckBox;
	CPUTCheckbox		  *mpPipelineCheckBox;

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

	AABBoxRasterizer				*mpAABB;
	AABBoxRasterizerScalarST		*mpAABBScalarST;
	AABBoxRasterizerScalarMT		*mpAABBScalarMT;
	AABBoxRasterizerSSEST			*mpAABBSSEST;
	AABBoxRasterizerSSEMT			*mpAABBSSEMT;
	AABBoxRasterizerAVXST			*mpAABBAVXST;
	AABBoxRasterizerAVXMT			*mpAABBAVXMT;
	AABBoxMaskedRasterizerAVXST		*mpAABBMAVXST;

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
	virtual void UpdateGPUDepthBuf(MaskedOcclusionCulling *moc);
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
};
#endif // __CPUT_SAMPLESTARTDX11_H__
