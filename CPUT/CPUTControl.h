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
#ifndef __CPUTCONTROL_H__
#define __CPUTCONTROL_H__

#include <stdio.h>

#include "CPUT.h"
#include "CPUTEventHandler.h"

// internal types
// TODO: Why are these int and not float?
struct CPUT_SIZE
{
    int width;
    int height;
};

struct CPUT_POINT
{
    int x;
    int y;
};

struct CPUT_RECT
{
    int x;
    int y;
    int width;
    int height;
};
// This is a list of all the known control types
// if you make a new control, be sure to add it here
// so the GUI controller can manage it properly
enum CPUTControlType
{
    CPUT_CONTROL_UNKNOWN,
    CPUT_BUTTON,
    CPUT_CHECKBOX,
    CPUT_DROPDOWN,
    CPUT_SLIDER,
    CPUT_STATIC,
    CPUT_PANE,
};

// state of the control
typedef enum CPUTGUIControlState
{
    CPUT_CONTROL_ACTIVE,
    CPUT_CONTROL_INACTIVE,
} CPUTGUIControlState;

// control/event ID defines
typedef unsigned int UINT;
typedef UINT CPUTControlID;
typedef UINT CPUTEventID;
const UINT CPUT_CONTROL_ID_INVALID=(UINT)-1;

// forward declaration
class CPUTCallbackHandler;

// CPUTControl base class
// This is basically a virtual class that defines the common calls for manipulation
// controls.  Individual controls implement this interface.
class CPUTControl:public CPUTEventHandler
{
public:
    CPUTControl();
    virtual ~CPUTControl();

    // Control type
    virtual void SetControlID(CPUTControlID id);
    virtual CPUTControlID GetControlID();
    virtual CPUTControlType GetType();

    // Set what object to call back on events
    void SetControlCallback(CPUTCallbackHandler *pHandler);

    // Graphical state manipulation
    virtual void GetPosition(int &x, int &y)=0;
    virtual void SetPosition(int x, int y)=0;
    virtual void GetDimensions(int &width, int &height)=0;
    virtual bool ContainsPoint(int x, int y)=0;
    virtual void SetVisibility(bool bVisible);
    virtual bool IsVisible();
    virtual void SetEnable(bool in_bEnabled);
    virtual bool IsEnabled();
    virtual void SetAutoArranged(bool bIsAutoArranged);
    virtual bool IsAutoArranged();

    // keyboard event hotkey for this control
    virtual void SetHotkey(CPUTKey hotKey);
    virtual CPUTKey GetHotkey();

    // buffer management
    virtual unsigned int GetQuadCount() {return 0;}
    virtual void DrawIntoBuffer(float *pVertexBufferMirror, int *pInsertIndex, int pMaxBufferSize) {return;}

protected:
    bool                    mControlVisible;
    bool                    mControlAutoArranged;
    CPUTKey                 mhotkey;
    CPUTControlType         mcontrolType;
    CPUTControlID           mcontrolID;
    CPUTCallbackHandler    *mpCallbackHandler;
    CPUTGUIControlState     mControlState;
};

#endif //#ifndef __CPUTCONTROL_H__
