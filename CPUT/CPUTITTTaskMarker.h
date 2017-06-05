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
#ifndef __CPUTITTTASKMARKER_H__
#define __CPUTITTTASKMARKER_H__



#include "CPUT.h"

#ifdef CPUT_GPA_INSTRUMENTATION

// GPA ITT instrumentation helper class - only available in profile build
// Automatically open/close marks an ITT marker event for GPA
//-----------------------------------------------------------------------------
class CPUTITTTaskMarker
{
public:
    CPUTITTTaskMarker(__itt_domain *pITTDomain, __itt_string_handle *pITTStringHandle);
    ~CPUTITTTaskMarker();
private:
    __itt_domain *mpITTDomain;
    __itt_string_handle *mpITTStringHandle;
};

#endif
#endif // #ifndef __CPUTITTTASKMARKER_H__