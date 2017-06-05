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
#ifndef _CPUTCOMPUTESHADERDX11_H
#define _CPUTCOMPUTESHADERDX11_H

#include "CPUT.h"
#include "CPUTShaderDX11.h"

class CPUTComputeShaderDX11 : public CPUTShaderDX11
{
protected:
    ID3D11ComputeShader *mpComputeShader;

     // Destructor is not public.  Must release instead of delete.
    ~CPUTComputeShaderDX11(){ SAFE_RELEASE(mpComputeShader) }

public:
    static CPUTComputeShaderDX11 *CreateComputeShader(
        const cString  &name,
        ID3D11Device   *pD3dDevice,
        const cString  &shaderMain,
        const cString  &shaderProfile
    );

    static CPUTComputeShaderDX11 *CreateComputeShaderFromMemory(
        const cString        &name,
        ID3D11Device         *pD3dDevice,
        const cString        &shaderMain,
        const cString        &shaderProfile,
        const char           *pShaderSource
    );
    CPUTComputeShaderDX11() : mpComputeShader(NULL), CPUTShaderDX11(NULL) {}
    CPUTComputeShaderDX11(ID3D11ComputeShader *pD3D11ComputeShader, ID3DBlob *pBlob) : mpComputeShader(pD3D11ComputeShader), CPUTShaderDX11(pBlob) {}
    ID3DBlob *GetBlob() { return mpBlob; }
    ID3D11ComputeShader *GetNativeComputeShader() { return mpComputeShader; }
};

#endif //_CPUTCOMPUTESHADER_H
