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
#ifndef __CPUTBUTTON_H__
#define __CPUTBUTTON_H__

#include "CPUTControl.h"
#include <string.h>
#include "CPUTGuiController.h"

class CPUTFont;
class CPUTText;

// default padding between controls
#define CPUT_BUTTON_TEXT_BORDER_PADDING_X 15
#define CPUT_BUTTON_TEXT_BORDER_PADDING_Y 5

typedef enum CPUTButtonState
{
    CPUT_BUTTON_NEUTRAL,
    CPUT_BUTTON_PRESSED,
} CPUTButtonState;

const int CPUT_NUM_IMAGES_IN_BUTTON=9;
const int CPUT_NUM_VERTS_IN_BUTTON_QUAD=6;

// Button base - common functionality for the control
//-----------------------------------------------------------------------------
class CPUTButton:public CPUTControl
{
public:
    // constructors
    CPUTButton(CPUTButton& copy); // don't allow copy construction
    CPUTButton(const cString ControlText, CPUTControlID id, CPUTFont *pFont);
    virtual ~CPUTButton();

    // CPUTControl
    virtual void GetPosition(int &x, int &y);
    virtual void GetDimensions(int &width, int &height);
    virtual bool ContainsPoint(int x, int y);
    virtual void SetPosition(int x, int y);    
    virtual void SetText(const cString String);
    virtual void GetText(cString &String);
    virtual unsigned int GetOutputVertexCount();
    virtual void SetEnable(bool in_bEnabled);
    
    //CPUTEventHandler
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);
      
    // Register assets
    static CPUTResult RegisterStaticResources();
    static CPUTResult UnRegisterStaticResources();

    CPUTResult RegisterInstanceResources();
    CPUTResult UnRegisterInstanceResources();

    // draw
    virtual void DrawIntoBuffer(CPUTGUIVertex *pVertexBufferMirror, UINT *pInsertIndex, UINT pMaxBufferSize, CPUTGUIVertex *pTextVertexBufferMirror, UINT *pTextInsertIndex, UINT MaxTextVertexBufferSize);
        
protected:
    CPUT_RECT mButtonDimensions;
    CPUTButtonState mButtonState;

    // helper functions
    // control state
    bool mbMouseInside;
    bool mbStartedClickInside;
    CPUTFont *mpFont;
    
    // Static resources
    static bool mStaticRegistered;

    // sizes of unmodified button graphics
    static CPUT_SIZE mpButtonIdleImageSizeList[CPUT_NUM_IMAGES_IN_BUTTON];
    static CPUT_SIZE mpButtonPressedImageSizeList[CPUT_NUM_IMAGES_IN_BUTTON];
    static CPUT_SIZE mpButtonDisabledImageSizeList[CPUT_NUM_IMAGES_IN_BUTTON];
    


    static int mSmallestLeftSizeIdle;
    static int mSmallestRightSizeIdle;
    static int mSmallestTopSizeIdle;
    static int mSmallestBottomSizeIdle;

    static int mSmallestLeftSizePressed;
    static int mSmallestRightSizePressed;
    static int mSmallestTopSizePressed;
    static int mSmallestBottomSizePressed;

    static int mSmallestLeftSizeDisabled;
    static int mSmallestRightSizeDisabled;
    static int mSmallestTopSizeDisabled;
    static int mSmallestBottomSizeDisabled;

    
    // per-instance information
    CPUTText *mpButtonText;
    CPUT_SIZE mpButtonIdleSizeList[CPUT_NUM_IMAGES_IN_BUTTON];
    CPUT_SIZE mpButtonPressedSizeList[CPUT_NUM_IMAGES_IN_BUTTON];
    CPUT_SIZE mpButtonDisabledSizeList[CPUT_NUM_IMAGES_IN_BUTTON];

    CPUTGUIVertex *mpMirrorBufferActive;
    CPUTGUIVertex *mpMirrorBufferPressed;
    CPUTGUIVertex *mpMirrorBufferDisabled;

    // helper functions   
    void InitializeState();
    void SetDimensions(int width, int height);
    CPUTResult Resize(int width, int height);
    void AddQuadIntoMirrorBuffer(CPUTGUIVertex *pMirrorBuffer, int index, float x, float y, float w, float h, float3 uv1, float3 uv2 );
    void GetInsetTextCoordinate(int &x, int &y);


};

#endif //#ifndef __CPUTBUTTON_H__