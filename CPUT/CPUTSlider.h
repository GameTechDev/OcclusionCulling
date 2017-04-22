/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CPUTSLIDER_H__
#define __CPUTSLIDER_H__

#include "CPUTControl.h"

struct CPUTGUIVertex;
class CPUTFont;
class CPUTText;

typedef enum CPUTSliderState
{
    CPUT_SLIDER_NIB_UNPRESSED,
    CPUT_SLIDER_NIB_PRESSED,
}CPUTSliderState;

#define CPUT_NUM_IMAGES_IN_SLIDER_ARRAY 6
#define CPUT_NUM_QUADS_IN_SLIDER_ARRAY 6
#define CPUT_DEFAULT_TRAY_WIDTH 198

// Button base - common functionality for all the controls
//-----------------------------------------------------------------------------
class CPUTSlider:public CPUTControl
{
protected:
    // control position/location/dimensionss
    CPUT_RECT mControlDimensions;

    // slider state/range
    float     mSliderStartValue;
    float     mSliderEndValue;
    int       mSliderNumberOfSteps;
    int       mSliderNumberOfTicks;
    float     mCurrentSliderValue;

    // others
    CPUTSliderState mSliderState;

    void   InitialStateSet();

public:
    // button should self-register with the GuiController on create
    CPUTSlider(CPUTSlider &copy);
    CPUTSlider(const cString ControlText, CPUTControlID id, CPUTFont *pFont);

    virtual ~CPUTSlider();

    //CPUTEventHandler
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key){UNREFERENCED_PARAMETER(key); return CPUT_EVENT_UNHANDLED;}
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);

    // CPUTControl  &graphical manipulation
    void GetPosition(int &x, int &y);
    void GetDimensions(int &width, int &height);
    void SetPosition(int x, int y);
    virtual bool ContainsPoint(int x, int y);
    void SetText(const cString ControlText);
    virtual void SetEnable(bool in_bEnabled);
    virtual unsigned int GetOutputVertexCount();

    //CPUTSlider
    CPUTResult SetScale(float StartValue, float EndValue, int NumberOfSteps);
    CPUTResult SetNumberOfTicks(int NumberOfTicks);
    CPUTResult GetValue(float  &fValue);
    CPUTResult SetValue(float fValue);
    void SetTickDrawing(bool DrawTicks);

    // Register assets
    static CPUTResult RegisterStaticResources();
    static CPUTResult UnRegisterStaticResources();

    CPUTResult RegisterInstanceResources();
    CPUTResult UnRegisterInstanceResources();
    
    // draw
    void DrawIntoBuffer(CPUTGUIVertex *pVertexBufferMirror, UINT *pInsertIndex, UINT pMaxBufferSize, CPUTGUIVertex *pTextVertexBufferMirror, UINT *pTextInsertIndex, UINT MaxTextVertexBufferSize);


protected:
        struct LocationGuides
    {
        float TickIndent;
        float TextDownIndent;
        float GripDownIndent;
        float TrayDownIndent;
        float TickSpacing;
        float StepSpacing;
        float TotalWidth;
        float TotalHeight;
    };

    CPUTText *mpControlText;
    CPUTFont *mpFont;
    int mSliderNubTickLocation;
    float mSliderNubLocation;
    bool mbDrawTicks;

    // events
    bool mbMouseInside;
    bool mbStartedClickInside;
    


    // uberbuffer    
    static CPUT_SIZE mpActiveImageSizeList[CPUT_NUM_IMAGES_IN_SLIDER_ARRAY];
    static CPUT_SIZE mpPressedImageSizeList[CPUT_NUM_IMAGES_IN_SLIDER_ARRAY];
    static CPUT_SIZE mpDisabledImageSizeList[CPUT_NUM_IMAGES_IN_SLIDER_ARRAY];

    CPUTGUIVertex *mpMirrorBufferActive;
    CPUTGUIVertex *mpMirrorBufferPressed;
    CPUTGUIVertex *mpMirrorBufferDisabled;
    void Recalculate();
    void AddQuadIntoMirrorBuffer(CPUTGUIVertex *pMirrorBuffer, int index, float x, float y, float w, float h, float3 uv1, float3 uv2 );    

    // helper functions
    bool PointingAtNub(int x, int y);
    void CalculateLocationGuides(LocationGuides &guides);
    void SnapToNearestTick(); 
    void ClampTicks();

};


#endif //#ifndef __CPUTSLIDER_H__