/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPUTVERTEXSHADERDX11_H
#define _CPUTVERTEXSHADERDX11_H

#include "CPUT.h"
#include "CPUTShaderDX11.h"

class CPUTVertexShaderDX11 : public CPUTShaderDX11
{
protected:
    ID3D11VertexShader *mpVertexShader;

    // Destructor is not public.  Must release instead of delete.
    ~CPUTVertexShaderDX11(){ SAFE_RELEASE(mpVertexShader) }

public:
    static CPUTVertexShaderDX11 *CreateVertexShader(
        const cString        &name,
        ID3D11Device         *pD3dDevice,
        const cString        &shaderMain,
        const cString        &shaderProfile
    );
static CPUTVertexShaderDX11 *CreateVertexShaderFromMemory(
        const cString        &name,
        ID3D11Device         *pD3dDevice,
        const cString        &shaderMain,
        const cString        &shaderProfile,
        const char           *pShaderSource
    );

    CPUTVertexShaderDX11() : mpVertexShader(NULL), CPUTShaderDX11(NULL) {}
    CPUTVertexShaderDX11(ID3D11VertexShader *pD3D11VertexShader, ID3DBlob *pBlob) : mpVertexShader(pD3D11VertexShader), CPUTShaderDX11(pBlob) {}
    ID3D11VertexShader *GetNativeVertexShader() { return mpVertexShader; }
};

#endif //_CPUTVERTEXSHADER_H
