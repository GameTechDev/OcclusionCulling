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
#ifndef _CPUTHULLSHADERDX11_H
#define _CPUTHULLSHADERDX11_H

#include "CPUT.h"
#include "CPUTShaderDX11.h"

class CPUTHullShaderDX11 : public CPUTShaderDX11
{
protected:
    ID3D11HullShader *mpHullShader;

    // Destructor is not public.  Must release instead of delete.
    ~CPUTHullShaderDX11(){ SAFE_RELEASE(mpHullShader); }

public:
    static CPUTHullShaderDX11 *CreateHullShader(
        const cString        &name,
        ID3D11Device         *pD3dDevice,
        const cString        &shaderMain,
        const cString        &shaderProfile
    );
    static CPUTHullShaderDX11 *CreateHullShaderFromMemory(
        const cString        &name,
        ID3D11Device         *pD3dDevice,
        const cString        &shaderMain,
        const cString        &shaderProfile,
        const char           *pShaderSource
    );


    CPUTHullShaderDX11() : mpHullShader(NULL), CPUTShaderDX11(NULL) {}
    CPUTHullShaderDX11(ID3D11HullShader *pD3D11HullShader, ID3DBlob *pBlob) : mpHullShader(pD3D11HullShader), CPUTShaderDX11(pBlob) {}
    ID3DBlob *GetBlob() { return mpBlob; }
    ID3D11HullShader *GetNativeHullShader() { return mpHullShader; }
};

#endif //_CPUTHULLSHADER_H
