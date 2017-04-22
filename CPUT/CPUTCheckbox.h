/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CPUTCHECKBOX_H__
#define __CPUTCHECKBOX_H__

#include "CPUTControl.h"
#include "CPUTGuiController.h"

// forward declarations
class CPUTFont;
class CPUTText;


typedef enum CPUTCheckboxState
{
    CPUT_CHECKBOX_UNCHECKED,
    CPUT_CHECKBOX_CHECKED,
} CPUTCheckboxState;

typedef enum CPUTCheckboxGUIState
{
    CPUT_CHECKBOX_GUI_NEUTRAL,
    CPUT_CHECKBOX_GUI_PRESSED,
} CPUTCheckboxGUIState;

const int CPUT_CHECKBOX_NUM_IMAGES_IN_CHECKBOX=3;
const int CPUT_CHECKBOX_PADDING=5;          // padding (in pixels) between checkbox image and the label

// Checkbox 
//-----------------------------------------------------------------------------
class CPUTCheckbox:public CPUTControl
{
public:
    // button should self-register with the GuiController on create
    CPUTCheckbox(CPUTCheckbox& copy);
    CPUTCheckbox(const cString ControlText, CPUTControlID id, CPUTFont *pFont);
    virtual ~CPUTCheckbox();

    // CPUTControl
    virtual void GetPosition(int &x, int &y);
    virtual unsigned int GetOutputVertexCount();

    // CPUTCheckboxDX11
    virtual CPUTCheckboxState GetCheckboxState();
    void SetCheckboxState(CPUTCheckboxState State);
    
    
    //CPUTEventHandler
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);

    //CPUTControl
    virtual bool ContainsPoint(int x, int y);
    virtual void SetPosition(int x, int y);
    virtual void GetDimensions(int &width, int &height);
    virtual void SetText(const cString String);
    virtual void GetText(cString &TextString);
    virtual void SetEnable(bool bEnabled);

    // Register assets
    static CPUTResult RegisterStaticResources();
    static CPUTResult UnRegisterStaticResources();

    CPUTResult RegisterInstanceResources();
    CPUTResult UnRegisterInstanceResources();

    // CPUTCheckboxDX11    
    void DrawIntoBuffer(CPUTGUIVertex *pVertexBufferMirror, UINT *pInsertIndex, UINT pMaxBufferSize, CPUTGUIVertex *pTextVertexBufferMirror, UINT *pTextInsertIndex, UINT MaxTextVertexBufferSize);


protected:
    CPUT_RECT               mControlDimensions;
    CPUTCheckboxState       mCheckboxState;
    CPUTCheckboxGUIState    mCheckboxGuiState;

    // helper functions
    void InitialStateSet();




    // new for uber-buffer version
    CPUTFont *mpFont;
    CPUTGUIVertex mpMirrorBufferActive[6];
    CPUTGUIVertex mpMirrorBufferPressed[6];
    CPUTGUIVertex mpMirrorBufferDisabled[6];
    void Recalculate();
    void AddQuadIntoMirrorBuffer(CPUTGUIVertex *pMirrorBuffer, float x, float y, float w, float h, float3 uv1, float3 uv2 );


    // static varibles used by ALL checkbox controls    
    static CPUT_SIZE mpCheckboxTextureSizeList[CPUT_CHECKBOX_NUM_IMAGES_IN_CHECKBOX];

    // GUI state
    bool mbMouseInside;
    bool mbStartedClickInside;

    // instance variables for this particular checkbox
    UINT mVertexStride;
    UINT mVertexOffset;    
    CPUTText *mpCheckboxText;

    // helper functions
    void GetTextPosition(int &x, int &y);
    void CalculateBounds();

};



#endif //#ifndef __CPUTCHECKBOX_H__