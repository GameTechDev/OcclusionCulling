/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUTPixelShaderDX11.h"
#include "CPUTAssetLibraryDX11.h"
#include "D3DCompiler.h"

CPUTPixelShaderDX11 *CPUTPixelShaderDX11::CreatePixelShader(
    const cString      &name,
    ID3D11Device       *pD3dDevice,
    const cString      &shaderMain,
    const cString      &shaderProfile
)
{
    ID3DBlob           *pCompiledBlob = NULL;
    ID3D11PixelShader  *pNewPixelShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling pixel shader:\n\n") );

    // Create the pixel shader
    // TODO: Move to pixel shader class
    HRESULT hr = pD3dDevice->CreatePixelShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewPixelShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating pixel shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetPixelShader ")+name;
    // CPUTSetDebugName(pNewPixelShader, DebugName);

    CPUTPixelShaderDX11 *pNewCPUTPixelShader = new CPUTPixelShaderDX11( pNewPixelShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddPixelShader(name + shaderMain + shaderProfile, pNewCPUTPixelShader);
    // pNewCPUTPixelShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTPixelShader;
}

//--------------------------------------------------------------------------------------
CPUTPixelShaderDX11 *CPUTPixelShaderDX11::CreatePixelShaderFromMemory(
    const cString      &name,
    ID3D11Device       *pD3dDevice,
    const cString      &shaderMain,
    const cString      &shaderProfile,
    const char         *pShaderSource
)
{
    ID3DBlob           *pCompiledBlob = NULL;
    ID3D11PixelShader  *pNewPixelShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromMemory(pShaderSource, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling pixel shader:\n\n") );

    // Create the pixel shader
    // TODO: Move to pixel shader class
    HRESULT hr = pD3dDevice->CreatePixelShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewPixelShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating pixel shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetPixelShader ")+name;
    // CPUTSetDebugName(pNewPixelShader, DebugName);

    CPUTPixelShaderDX11 *pNewCPUTPixelShader = new CPUTPixelShaderDX11( pNewPixelShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddPixelShader(name + shaderMain + shaderProfile, pNewCPUTPixelShader);
    // pNewCPUTPixelShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTPixelShader;
}
