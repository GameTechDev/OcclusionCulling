/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUTHullShaderDX11.h"
#include "CPUTAssetLibraryDX11.h"

CPUTHullShaderDX11 *CPUTHullShaderDX11::CreateHullShader(
    const cString     &name,
    ID3D11Device      *pD3dDevice,
    const cString     &shaderMain,
    const cString     &shaderProfile
)
{
    ID3DBlob          *pCompiledBlob = NULL;
    ID3D11HullShader  *pNewHullShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling Hull shader:\n\n") );

    // Create the Hull shader
    // TODO: Move to Hull shader class
    HRESULT hr = pD3dDevice->CreateHullShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewHullShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating Hull shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetHullShader ")+name;
    // CPUTSetDebugName(pNewHullShader, DebugName);

    CPUTHullShaderDX11 *pNewCPUTHullShader = new CPUTHullShaderDX11( pNewHullShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddHullShader(name + shaderMain + shaderProfile, pNewCPUTHullShader);
    // pNewCPUTHullShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTHullShader;
}

//--------------------------------------------------------------------------------------
CPUTHullShaderDX11 *CPUTHullShaderDX11::CreateHullShaderFromMemory(
    const cString     &name,
    ID3D11Device      *pD3dDevice,
    const cString     &shaderMain,
    const cString     &shaderProfile,
    const char        *pShaderSource
)
{
    ID3DBlob          *pCompiledBlob = NULL;
    ID3D11HullShader  *pNewHullShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromMemory(pShaderSource, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling Hull shader:\n\n") );

    // Create the Hull shader
    // TODO: Move to Hull shader class
    HRESULT hr = pD3dDevice->CreateHullShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewHullShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating Hull shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetHullShader ")+name;
    // CPUTSetDebugName(pNewHullShader, DebugName);

    CPUTHullShaderDX11 *pNewCPUTHullShader = new CPUTHullShaderDX11( pNewHullShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddHullShader(name + shaderMain + shaderProfile, pNewCPUTHullShader);
    // pNewCPUTHullShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTHullShader;
}
