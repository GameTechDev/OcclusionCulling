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
#ifndef __CPUTTEXT_H__
#define __CPUTTEXT_H__

#include "CPUTControl.h"
#include "CPUTGuiController.h"

class CPUTFont;

// Button base - common functionality for all the controls
//-----------------------------------------------------------------------------
class CPUTText:public CPUTControl
{
public:
    // button should self-register with the GuiController on create
    CPUTText(CPUTFont *pFont);
    CPUTText(CPUTText& copy);
    CPUTText(const cString String, CPUTControlID id, CPUTFont *pFont);

    virtual ~CPUTText();

    //Management
    virtual void GetString(cString &ButtonText);

    //CPUTEventHandler
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key){UNREFERENCED_PARAMETER(key); return CPUT_EVENT_UNHANDLED;}
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state){UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);UNREFERENCED_PARAMETER(wheel);UNREFERENCED_PARAMETER(state);return CPUT_EVENT_UNHANDLED;}
    
    //CPUTControl
    void GetDimensions(int &width, int &height);
    void GetPosition(int &x, int &y);    

    // CPUTControl
    virtual void SetPosition(int x, int y);
    void SetEnable(bool in_bEnabled);
    bool ContainsPoint(int x, int y) {UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);return false;}

    // CPUTText
    CPUTResult SetText(const cString String, float depth=0.5f);
    int GetOutputVertexCount();

    // Register assets
    CPUTResult RegisterInstanceData();
    static CPUTResult RegisterStaticResources();
    static CPUTResult UnRegisterStaticResources();

    // draw
    void DrawIntoBuffer(CPUTGUIVertex *pVertexBufferMirror, UINT *pInsertIndex, UINT pMaxBufferSize);


protected:
    // instance variables
    CPUT_POINT          mPosition;
    CPUT_RECT           mDimensions;
    CPUT_SIZE           mQuadSize;
    CPUTGUIControlState mStaticState;
        
    UINT mVertexStride;
    UINT mVertexOffset;
    CPUTFont           *mpFont;

    // uber-buffer
    cString mStaticText;
    float mZDepth;
    CPUTGUIVertex *mpMirrorBuffer;
    int mNumCharsInString;
    static CPUT_SIZE mpStaticIdleImageSizeList[500]; // todo: size for #chars in font
    static CPUT_SIZE mpStaticDisabledImageSizeList[500]; // todo: size for #chars in font
    

    // helper functions
    void InitialStateSet();
    void ReleaseInstanceData();
    void Recalculate();
    void AddQuadIntoMirrorBuffer(CPUTGUIVertex *pMirrorBuffer, int index, float x, float y, float w, float h, float3 uv1, float3 uv2 );

};




#endif //#ifndef __CPUTTEXT_H__