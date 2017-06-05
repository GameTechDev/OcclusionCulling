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
#ifndef _CPUTPIXELSHADERDX11_H
#define _CPUTPIXELSHADERDX11_H

#include "CPUT.h"
#include "CPUTShaderDX11.h"

class CPUTPixelShaderDX11 : public CPUTShaderDX11
{
protected:
    ID3D11PixelShader *mpPixelShader;

     // Destructor is not public.  Must release instead of delete.
    ~CPUTPixelShaderDX11(){ SAFE_RELEASE(mpPixelShader); }

public:
    static CPUTPixelShaderDX11 *CreatePixelShader(
        const cString        &name,
        ID3D11Device         *pD3dDevice,
        const cString        &shaderMain,
        const cString        &shaderProfile
    );

    static CPUTPixelShaderDX11 *CreatePixelShaderFromMemory(
        const cString        &name,
        ID3D11Device         *pD3dDevice,
        const cString        &shaderMain,
        const cString        &shaderProfile,
        const char           *pShaderSource
    );

    CPUTPixelShaderDX11() : mpPixelShader(NULL), CPUTShaderDX11(NULL) {}
    CPUTPixelShaderDX11(ID3D11PixelShader *pD3D11PixelShader, ID3DBlob *pBlob) : mpPixelShader(pD3D11PixelShader), CPUTShaderDX11(pBlob) {}
    ID3D11PixelShader *GetNativePixelShader() { return mpPixelShader; }
};

#endif //_CPUTPIXELSHADER_H
