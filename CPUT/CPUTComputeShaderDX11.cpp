/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUTComputeShaderDX11.h"
#include "CPUTAssetLibraryDX11.h"

CPUTComputeShaderDX11 *CPUTComputeShaderDX11::CreateComputeShader(
    const cString  &name,
    ID3D11Device   *pD3dDevice,
    const cString  &shaderMain,
    const cString  &shaderProfile
)
{
    ID3DBlob            *pCompiledBlob = NULL;
    ID3D11ComputeShader *pNewComputeShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling compute shader:\n\n") );

    // Create the compute shader
    // TODO: Move to compute shader class
    HRESULT hr = pD3dDevice->CreateComputeShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewComputeShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating compute shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetComputeShader ")+name;
    // CPUTSetDebugName(pNewComputeShader, DebugName);

    CPUTComputeShaderDX11 *pNewCPUTComputeShader = new CPUTComputeShaderDX11( pNewComputeShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddComputeShader(name + shaderMain + shaderProfile, pNewCPUTComputeShader);

    // return the shader
    return pNewCPUTComputeShader;
}

//--------------------------------------------------------------------------------------
CPUTComputeShaderDX11 *CPUTComputeShaderDX11::CreateComputeShaderFromMemory(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    const char           *pShaderSource
)
{
    ID3DBlob*           pCompiledBlob = NULL;
    ID3D11ComputeShader*  pNewComputeShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromMemory(pShaderSource, shaderMain, shaderProfile, &pCompiledBlob);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling Compute shader:\n\n") );

    // Create the Compute shader
    // TODO: Move to Compute shader class
    HRESULT hr = pD3dDevice->CreateComputeShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewComputeShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating Compute shader:\n\n") );
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetComputeShader ")+name;
    // CPUTSetDebugName(pNewComputeShader, DebugName);

    CPUTComputeShaderDX11 *pNewCPUTComputeShader = new CPUTComputeShaderDX11( pNewComputeShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddComputeShader(name + shaderMain + shaderProfile, pNewCPUTComputeShader);
    // pNewCPUTComputeShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTComputeShader;
}
