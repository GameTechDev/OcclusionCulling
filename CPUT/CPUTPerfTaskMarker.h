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
#ifndef __CPUTPERFTASKMARKER_H__
#define __CPUTPERFTASKMARKER_H__



#include "CPUT.h"


#ifdef CPUT_GPA_INSTRUMENTATION

// GPA instrumentation helper class - only available in profile build
// Allows you to easily add 'task markers' to certain events
//-----------------------------------------------------------------------------
class CPUTPerfTaskMarker
{
public:
    CPUTPerfTaskMarker(DWORD color, wchar_t *pString);
    ~CPUTPerfTaskMarker();
private:

};
#endif

#endif // #ifndef __CPUTPERFTASKMARKER_H__