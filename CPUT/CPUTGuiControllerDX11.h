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
#ifndef __CPUTGUICONTROLLERDX11_H__
#define __CPUTGUICONTROLLERDX11_H__

#include "CPUTGuiController.h"
#include "CPUTTimerWin.h"

#include "CPUTButton.h"
#include "CPUTText.h"
#include "CPUTCheckbox.h"
#include "CPUTSlider.h"
#include "CPUTDropdown.h"
#include "CPUTVertexShaderDX11.h"
#include "CPUTPixelShaderDX11.h"
#include "CPUTRenderStateBlockDX11.h"

//#define SAVE_RESTORE_DS_HS_GS_SHADER_STATE

// forward declarations
class CPUT_DX11;
class CPUTButton;
class CPUTSlider;
class CPUTCheckbox;
class CPUTDropdown;
class CPUTText;
class CPUTTextureDX11;
class CPUTFontDX11;

const unsigned int CPUT_GUI_BUFFER_SIZE = 5000;         // size (in number of verticies) for all GUI control graphics
const unsigned int CPUT_GUI_BUFFER_STRING_SIZE = 5000;  // size (in number of verticies) for all GUI string graphics
const unsigned int CPUT_GUI_VERTEX_BUFFER_SIZE = 5000;
const CPUTControlID ID_CPUT_GUI_FPS_COUNTER = 4000000201;        // pick very random number for FPS counter string ID

#include <d3d11.h>
#include <DirectXMath.h>

const int AVG_FRAMES = 150;

// the GUI controller class that dispatches the rendering calls to all the buttons
//--------------------------------------------------------------------------------
__declspec(align(16))
class CPUTGuiControllerDX11:public CPUTGuiController
{ 
    struct GUIConstantBufferVS
    {
	    DirectX::XMMATRIX Projection;
	    DirectX::XMMATRIX Model;
    };


public:

	void* operator new(size_t i) {
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p) {
		_mm_free(p);
	}

    static CPUTGuiControllerDX11 *GetController();
    static CPUTResult DeleteController();

    // initialization
    CPUTResult Initialize(ID3D11DeviceContext *pImmediateContext, cString &ResourceDirectory);
	CPUTResult ReleaseResources();
	

    // Control creation/deletion 'helpers'
    CPUTResult CreateButton(const cString pButtonText, CPUTControlID controlID, CPUTControlID panelID, CPUTButton **ppButton=NULL);
    CPUTResult CreateSlider(const cString pSliderText, CPUTControlID controlID, CPUTControlID panelID, CPUTSlider **ppSlider=NULL);
    CPUTResult CreateCheckbox(const cString pCheckboxText, CPUTControlID controlID, CPUTControlID panelID, CPUTCheckbox **ppCheckbox=NULL);
    CPUTResult CreateDropdown(const cString pSelectionText, CPUTControlID controlID, CPUTControlID panelID, CPUTDropdown **ppDropdown=NULL);
    CPUTResult CreateText(const cString Text,  CPUTControlID controlID, CPUTControlID panelID, CPUTText **ppStatic=NULL);    
    CPUTResult DeleteControl(CPUTControlID controlID);

    // draw routines    
    void Draw(ID3D11DeviceContext *pImmediateContext);
    void DrawFPS(bool drawfps);
    float GetFPS();

private:
    static CPUTGuiControllerDX11 *mguiController; // singleton object

    // DirectX state objects for GUI drawing
    CPUTVertexShaderDX11 *mpGUIVertexShader;
    CPUTPixelShaderDX11  *mpGUIPixelShader;
    ID3D11InputLayout    *mpVertexLayout;
    ID3D11Buffer         *mpConstantBufferVS;
    GUIConstantBufferVS   mModelViewMatrices;

    // Texture atlas
    CPUTTextureDX11            *mpControlTextureAtlas;
    ID3D11ShaderResourceView   *mpControlTextureAtlasView;
    ID3D11Buffer               *mpUberBuffer;
    CPUTGUIVertex              *mpMirrorBuffer;
    UINT                        mUberBufferIndex;
    UINT                        mUberBufferMax;
    
    // Font atlas
    CPUTFontDX11               *mpFont;
    CPUTTextureDX11            *mpTextTextureAtlas;
    ID3D11ShaderResourceView   *mpTextTextureAtlasView;
    ID3D11Buffer               *mpTextUberBuffer;
    CPUTGUIVertex              *mpTextMirrorBuffer;
    UINT                        mTextUberBufferIndex;

    // Focused control buffers
    CPUTGUIVertex              *mpFocusedControlMirrorBuffer;
    UINT                        mFocusedControlBufferIndex;
    ID3D11Buffer               *mpFocusedControlBuffer;
    CPUTGUIVertex              *mpFocusedControlTextMirrorBuffer;
    UINT                        mFocusedControlTextBufferIndex;
    ID3D11Buffer               *mpFocusedControlTextBuffer;



    // FPS
    bool                        mbDrawFPS;
    float                       mLastFPS;
    CPUTText                   *mpFPSCounter;
    // FPS control buffers
    CPUTGUIVertex              *mpFPSMirrorBuffer;
    UINT                        mFPSBufferIndex;
    ID3D11Buffer               *mpFPSDirectXBuffer;
    CPUTTimerWin               *mpFPSTimer;

    // render state
    CPUTRenderStateBlockDX11   *mpGUIRenderStateBlock;
    CPUTResult UpdateUberBuffers(ID3D11DeviceContext *pImmediateContext );

#ifdef SAVE_RESTORE_DS_HS_GS_SHADER_STATE
    ID3D11GeometryShader   *mpGeometryShaderState;
    ID3D11ClassInstance    *mpGeometryShaderClassInstances;
    UINT                    mGeometryShaderNumClassInstances;

    ID3D11HullShader       *mpHullShaderState;
    ID3D11ClassInstance    *mpHullShaderClassInstances;
    UINT                    mHullShaderNumClassInstance;

    ID3D11DomainShader     *mpDomainShaderState;
    ID3D11ClassInstance    *mpDomainShaderClassIntances;
    UINT                    mDomainShaderNumClassInstances;
#endif


    // members for saving render state before/after drawing gui
    D3D11_PRIMITIVE_TOPOLOGY    mTopology;
	float mFPSAvg[AVG_FRAMES];
	UINT mFPSInst;

    // helper functions
    CPUTGuiControllerDX11();    // singleton
    ~CPUTGuiControllerDX11();
    CPUTResult RegisterGUIResources(ID3D11DeviceContext *pImmediateContext, cString VertexShaderFilename, cString RenderStateFile, cString PixelShaderFilename, cString DefaultFontFilename, cString ControlTextureAtlas);
    void SetGUIDrawingState(ID3D11DeviceContext *pImmediateContext);
    void ClearGUIDrawingState(ID3D11DeviceContext *pImmediateContext);
};




#endif // #ifndef __CPUTGUICONTROLLERDX11_H__
