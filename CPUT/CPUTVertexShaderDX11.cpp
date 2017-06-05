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

#include "CPUTVertexShaderDX11.h"
#include "CPUTAssetLibraryDX11.h"

CPUTVertexShaderDX11 *CPUTVertexShaderDX11::CreateVertexShader(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile
)
{
    ID3DBlob            *pCompiledBlob = NULL;
    ID3D11VertexShader  *pNewVertexShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling vertex shader:\n\n") );

    // Create the vertex shader
    // TODO: Move to vertex shader class
    HRESULT hr = pD3dDevice->CreateVertexShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewVertexShader );
    ASSERT( SUCCEEDED(result), _L("Error compiling vertex shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetVertexShader ")+name;
    // CPUTSetDebugName(pNewVertexShader, DebugName);

    CPUTVertexShaderDX11 *pNewCPUTVertexShader = new CPUTVertexShaderDX11( pNewVertexShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddVertexShader(name + shaderMain + shaderProfile, pNewCPUTVertexShader);
    // pNewCPUTVertexShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTVertexShader;
}

//--------------------------------------------------------------------------------------
CPUTVertexShaderDX11 *CPUTVertexShaderDX11::CreateVertexShaderFromMemory(
    const cString      &name,
    ID3D11Device       *pD3dDevice,
    const cString      &shaderMain,
    const cString      &shaderProfile,
    const char         *pShaderSource
)
{
    ID3DBlob           *pCompiledBlob = NULL;
    ID3D11VertexShader *pNewVertexShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromMemory(pShaderSource, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling vertex shader:\n\n") );

    // Create the vertex shader
    // TODO: Move to vertex shader class
    HRESULT hr = pD3dDevice->CreateVertexShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewVertexShader );
    ASSERT( SUCCEEDED(result), _L("Error compiling vertex shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetVertexShader ")+name;
    // CPUTSetDebugName(pNewVertexShader, DebugName);

    CPUTVertexShaderDX11 *pNewCPUTVertexShader = new CPUTVertexShaderDX11( pNewVertexShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddVertexShader(name + shaderMain + shaderProfile, pNewCPUTVertexShader);
    // pNewCPUTVertexShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTVertexShader;
}
