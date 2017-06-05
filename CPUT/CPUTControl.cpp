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
#include "CPUTControl.h"

// Constructor
//------------------------------------------------------------------------------
CPUTControl::CPUTControl():mControlVisible(true),
    mControlAutoArranged(true),
    mhotkey(KEY_NONE),
    mcontrolType(CPUT_CONTROL_UNKNOWN),
    mcontrolID(0),
    mpCallbackHandler(NULL),
    mControlState(CPUT_CONTROL_ACTIVE)
{
}

// Destructor
//------------------------------------------------------------------------------
CPUTControl::~CPUTControl()
{
}

// Control type/identifier routines that have a common implementation for all controls

// Sets the control's ID used for identification purposes (hopefully unique)
//------------------------------------------------------------------------------
void CPUTControl::SetControlID(CPUTControlID id)
{
    mcontrolID = id;
}

// Get the ID for this control
//------------------------------------------------------------------------------
CPUTControlID CPUTControl::GetControlID()
{
    return mcontrolID;
}

// Get the type of control this is (button/dropdown/etc)
//------------------------------------------------------------------------------
CPUTControlType CPUTControl::GetType()
{
    return mcontrolType;
}


// Set callback handler
//------------------------------------------------------------------------------
void CPUTControl::SetControlCallback(CPUTCallbackHandler *pHandler)
{
    mpCallbackHandler = pHandler;
}


// set whether controls is visible or not (it is still there, but not visible)
//------------------------------------------------------------------------------
void CPUTControl::SetVisibility(bool bVisible)
{
    mControlVisible = bVisible;
}

// visibility state
//------------------------------------------------------------------------------
bool CPUTControl::IsVisible()
{
    return mControlVisible;
}

// Set the hot key for keyboard events for this control
//------------------------------------------------------------------------------
void CPUTControl::SetHotkey(CPUTKey hotKey)
{
    mhotkey = hotKey;
}

// Get the hot key set for this control
//------------------------------------------------------------------------------
CPUTKey CPUTControl::GetHotkey()
{
    return mhotkey;
}

// Should this control be auto-arranged?
//------------------------------------------------------------------------------
void CPUTControl::SetAutoArranged(bool bIsAutoArranged)
{
    mControlAutoArranged = bIsAutoArranged;
}

//------------------------------------------------------------------------------
bool CPUTControl::IsAutoArranged()
{
    return mControlAutoArranged;
}

// Set the control to enabled or greyed out
//------------------------------------------------------------------------------
void CPUTControl::SetEnable(bool bEnabled)
{
    if(!bEnabled)
    {
        mControlState = CPUT_CONTROL_INACTIVE;
    }
    else
    {
        mControlState = CPUT_CONTROL_ACTIVE;
    }
}

// Return bool if the control is enabled/greyed out
//------------------------------------------------------------------------------
bool CPUTControl::IsEnabled()
{
    if(mControlState == CPUT_CONTROL_INACTIVE)
    {
        return false;
    }

    return true;
}