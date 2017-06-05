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


#include "CPUTGeometryShaderDX11.h"
#include "CPUTAssetLibraryDX11.h"

CPUTGeometryShaderDX11 *CPUTGeometryShaderDX11::CreateGeometryShader(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile
)
{
    ID3DBlob             *pCompiledBlob = NULL;
    ID3D11GeometryShader *pNewGeometryShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling Geometry shader:\n\n") );

    // Create the Geometry shader
    // TODO: Move to Geometry shader class
    HRESULT hr = pD3dDevice->CreateGeometryShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewGeometryShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating Geometry shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetGeometryShader ")+name;
    // CPUTSetDebugName(pNewGeometryShader, DebugName);

    CPUTGeometryShaderDX11 *pNewCPUTGeometryShader = new CPUTGeometryShaderDX11( pNewGeometryShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddGeometryShader(name + shaderMain + shaderProfile, pNewCPUTGeometryShader);
    // pNewCPUTGeometryShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTGeometryShader;
}

//--------------------------------------------------------------------------------------
CPUTGeometryShaderDX11 *CPUTGeometryShaderDX11::CreateGeometryShaderFromMemory(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    const char           *pShaderSource
)
{
    ID3DBlob             *pCompiledBlob = NULL;
    ID3D11GeometryShader *pNewGeometryShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromMemory(pShaderSource, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error creating Geometry shader:\n\n") );

    // Create the Geometry shader
    // TODO: Move to Geometry shader class
    HRESULT hr = pD3dDevice->CreateGeometryShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewGeometryShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating Geometry shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetGeometryShader ")+name;
    // CPUTSetDebugName(pNewGeometryShader, DebugName);

    CPUTGeometryShaderDX11 *pNewCPUTGeometryShader = new CPUTGeometryShaderDX11( pNewGeometryShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddGeometryShader(name + shaderMain + shaderProfile, pNewCPUTGeometryShader);
    // pNewCPUTGeometryShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTGeometryShader;
}
