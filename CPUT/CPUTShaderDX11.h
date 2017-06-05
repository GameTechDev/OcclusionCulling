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
#ifndef _CPUTSHADERDX11_H
#define _CPUTSHADERDX11_H

#include "CPUT.h"
#include "CPUTRefCount.h"

class CPUTConfigBlock;

class CPUTShaderDX11 : public CPUTRefCount
{
protected:
    ID3DBlob          *mpBlob;

     // Destructor is not public.  Must release instead of delete.
    ~CPUTShaderDX11(){ SAFE_RELEASE(mpBlob); }

public:
    CPUTShaderDX11() : mpBlob(NULL) {}
    CPUTShaderDX11(ID3DBlob *pBlob) : mpBlob(pBlob) {}
    ID3DBlob *GetBlob() { return mpBlob; }

    bool ShaderRequiresPerModelPayload( CPUTConfigBlock &properties );
};

#endif //_CPUTPIXELSHADER_H
