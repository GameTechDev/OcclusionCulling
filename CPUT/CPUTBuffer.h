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
#ifndef _CPUTBUFFER_H
#define _CPUTBUFFER_H

#include "CPUT.h"
#include "CPUTRefCount.h"

// TODO: Move to dedicated file
class CPUTBuffer : public CPUTRefCount
{
protected:
    cString      mName;
    eCPUTMapType mMappedType;

    ~CPUTBuffer(){
        // mName.clear();
    } // Destructor is not public.  Must release instead of delete.
public:
    CPUTBuffer(){mMappedType = CPUT_MAP_UNDEFINED;}
    CPUTBuffer(cString &name) {mName = name; mMappedType = CPUT_MAP_UNDEFINED;}
    const cString &GetName() { return mName; }
};

#endif //_CPUTBUFFER_H
