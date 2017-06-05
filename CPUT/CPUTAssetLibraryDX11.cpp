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
#include "CPUTAssetLibraryDX11.h"

// define the objects we'll need
#include "CPUTModelDX11.h"
#include "CPUTMaterialDX11.h"
#include "CPUTTextureDX11.h"
#include "CPUTRenderStateBlockDX11.h"
#include "CPUTLight.h"
#include "CPUTCamera.h"
#include "CPUTVertexShaderDX11.h"
#include "CPUTPixelShaderDX11.h"
#include "CPUTGeometryShaderDX11.h"
#include "CPUTComputeShaderDX11.h"
#include "CPUTHullShaderDX11.h"
#include "CPUTDomainShaderDX11.h"

// MPF: opengl es - yipe - can't do both at the same time - need to have it bind dynamically/via compile-time 
CPUTAssetLibrary   *CPUTAssetLibrary::mpAssetLibrary = new CPUTAssetLibraryDX11();
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpPixelShaderList    = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpComputeShaderList  = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpVertexShaderList   = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpGeometryShaderList = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpHullShaderList = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpDomainShaderList = NULL;

CPUTAssetListEntry *CPUTAssetLibraryDX11::mpPixelShaderListTail    = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpComputeShaderListTail  = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpVertexShaderListTail   = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpGeometryShaderListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpHullShaderListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpDomainShaderListTail = NULL;

// TODO: Change OS Services to a flat list of CPUT* functions.  Avoid calls all over the place like:
// CPUTOSServices::GetOSServices();

// Deletes and properly releases all asset library lists that contain
// unwrapped IUnknown DirectX objects.
//-----------------------------------------------------------------------------
void CPUTAssetLibraryDX11::ReleaseAllLibraryLists()
{
    // TODO: we really need to wrap the DX assets so we don't need to distinguish their IUnknown type.
    SAFE_RELEASE_LIST(mpPixelShaderList);
    SAFE_RELEASE_LIST(mpComputeShaderList);
    SAFE_RELEASE_LIST(mpVertexShaderList);
    SAFE_RELEASE_LIST(mpGeometryShaderList);
    SAFE_RELEASE_LIST(mpHullShaderList);
    SAFE_RELEASE_LIST(mpDomainShaderList);

    // Call base class implementation to clean up the non-DX object lists
    return CPUTAssetLibrary::ReleaseAllLibraryLists();
}

// Erase the specified list, Release()-ing underlying objects
//-----------------------------------------------------------------------------
void CPUTAssetLibraryDX11::ReleaseIunknownList( CPUTAssetListEntry *pList )
{
    CPUTAssetListEntry *pNode = pList;
    CPUTAssetListEntry *pOldNode = NULL;

    while( NULL!=pNode )
    {
        // release the object using the DirectX IUnknown interface
        ((IUnknown*)(pNode->pData))->Release();
        pOldNode = pNode;
        pNode = pNode->pNext;
        delete pOldNode;
    }
    HEAPCHECK;
}

// Retrieve specified pixel shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetPixelShader(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTPixelShaderDX11 **ppPixelShader,
    bool                  nameIsFullPathAndFilename
)
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindPixelShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppPixelShader = (CPUTPixelShaderDX11*) pShader;
        (*ppPixelShader)->AddRef();
        return result;
    }
    *ppPixelShader = CPUTPixelShaderDX11::CreatePixelShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;
}

// Retrieve specified pixel shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetComputeShader(
    const cString          &name,
    ID3D11Device           *pD3dDevice,
    const cString          &shaderMain,
    const cString          &shaderProfile,
    CPUTComputeShaderDX11 **ppComputeShader,
    bool                    nameIsFullPathAndFilename
)
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices* pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindComputeShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppComputeShader = (CPUTComputeShaderDX11*) pShader;
        (*ppComputeShader)->AddRef();
        return result;
    }
    *ppComputeShader = CPUTComputeShaderDX11::CreateComputeShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;
}

// Retrieve specified vertex shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetVertexShader(
    const cString         &name,
    ID3D11Device          *pD3dDevice,
    const cString          &shaderMain,
    const cString          &shaderProfile,
    CPUTVertexShaderDX11 **ppVertexShader,
    bool                   nameIsFullPathAndFilename
)
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindVertexShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppVertexShader = (CPUTVertexShaderDX11*) pShader;
        (*ppVertexShader)->AddRef();
        return result;
    }
    *ppVertexShader = CPUTVertexShaderDX11::CreateVertexShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;
}

// Retrieve specified geometry shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetGeometryShader(
    const cString           &name,
    ID3D11Device            *pD3dDevice,
    const cString           &shaderMain,
    const cString           &shaderProfile,
    CPUTGeometryShaderDX11 **ppGeometryShader,
    bool                     nameIsFullPathAndFilename
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindGeometryShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppGeometryShader = (CPUTGeometryShaderDX11*) pShader;
        (*ppGeometryShader)->AddRef();
        return result;
    }
    *ppGeometryShader = CPUTGeometryShaderDX11::CreateGeometryShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;
}

// Retrieve specified hull shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetHullShader(
    const cString           &name,
    ID3D11Device            *pD3dDevice,
    const cString           &shaderMain,
    const cString           &shaderProfile,
    CPUTHullShaderDX11 **ppHullShader,
    bool                     nameIsFullPathAndFilename
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindHullShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppHullShader = (CPUTHullShaderDX11*) pShader;
        (*ppHullShader)->AddRef();
        return result;
    }
    *ppHullShader = CPUTHullShaderDX11::CreateHullShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;

}

// Retrieve specified domain shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetDomainShader(
    const cString           &name,
    ID3D11Device            *pD3dDevice,
    const cString           &shaderMain,
    const cString           &shaderProfile,
    CPUTDomainShaderDX11 **ppDomainShader,
    bool                     nameIsFullPathAndFilename
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindDomainShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppDomainShader = (CPUTDomainShaderDX11*) pShader;
        (*ppDomainShader)->AddRef();
        return result;
    }
    *ppDomainShader = CPUTDomainShaderDX11::CreateDomainShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;
}


//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreatePixelShaderFromMemory(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTPixelShaderDX11 **ppShader,
    char                 *pShaderSource
)
{
    CPUTResult result = CPUT_SUCCESS;
    void *pShader = FindPixelShader(name + shaderMain + shaderProfile, true);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
    *ppShader = CPUTPixelShaderDX11::CreatePixelShaderFromMemory( name, pD3dDevice, shaderMain, shaderProfile, pShaderSource);
    return result;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreateVertexShaderFromMemory(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTVertexShaderDX11 **ppShader,
    char                 *pShaderSource
)
{
    CPUTResult result = CPUT_SUCCESS;
    void *pShader = FindPixelShader(name + shaderMain + shaderProfile, true);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
    *ppShader = CPUTVertexShaderDX11::CreateVertexShaderFromMemory( name, pD3dDevice, shaderMain, shaderProfile, pShaderSource);
    return result;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreateComputeShaderFromMemory(
    const cString          &name,
    ID3D11Device           *pD3dDevice,
    const cString          &shaderMain,
    const cString          &shaderProfile,
    CPUTComputeShaderDX11 **ppShader,
    char                   *pShaderSource
)
{
    CPUTResult result = CPUT_SUCCESS;
    void *pShader = FindPixelShader(name + shaderMain + shaderProfile, true);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
    *ppShader = CPUTComputeShaderDX11::CreateComputeShaderFromMemory( name, pD3dDevice, shaderMain, shaderProfile, pShaderSource);
    return result;
}

// Use DX11 compile from file method to do all the heavy lifting
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CompileShaderFromFile(
    const cString  &fileName,
    const cString  &shaderMain,
    const cString  &shaderProfile,
    ID3DBlob      **ppBlob
)
{
    CPUTResult result = CPUT_SUCCESS;

    char pShaderMainAsChar[128];
    char pShaderProfileAsChar[128];
    ASSERT( shaderMain.length()     < 128, _L("Shader main name '")    + shaderMain    + _L("' longer than 128 chars.") );
    ASSERT( shaderProfile.length()  < 128, _L("Shader profile name '") + shaderProfile + _L("' longer than 128 chars.") );
    size_t count;
    wcstombs_s( &count, pShaderMainAsChar,    shaderMain.c_str(),    128 );
    wcstombs_s( &count, pShaderProfileAsChar, shaderProfile.c_str(), 128 );

    // use DirectX to compile the shader file
    ID3DBlob *pErrorBlob = NULL;
    D3D10_SHADER_MACRO pShaderMacros[2] = { "_CPUT", "1", NULL, NULL };

    HRESULT hr = D3DCompileFromFile(
        fileName.c_str(),     // fileName
        pShaderMacros,        // macro define's
        D3D_COMPILE_STANDARD_FILE_INCLUDE,                 // includes
        pShaderMainAsChar,    // main function name
        pShaderProfileAsChar, // shader profile/feature level
        0,                    // flags 1
        0,                    // flags 2
        ppBlob,               // blob data with compiled code
        &pErrorBlob           // any compile errors stored here
    );
    ASSERT( SUCCEEDED(hr), _L("Error compiling shader '") + fileName + _L("'.\n") + (pErrorBlob ? s2ws((char*)pErrorBlob->GetBufferPointer()) : _L("no error message") ) );
    if(pErrorBlob)
    {
        pErrorBlob->Release();
    }
    return result;
}

// Use DX11 compile from file method to do all the heavy lifting
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CompileShaderFromMemory(
    const char     *pShaderSource,
    const cString  &shaderMain,
    const cString  &shaderProfile,
    ID3DBlob      **ppBlob
)
{
    CPUTResult result = CPUT_SUCCESS;

    char pShaderMainAsChar[128];
    char pShaderProfileAsChar[128];
    ASSERT( shaderMain.length()     < 128, _L("Shader main name '")    + shaderMain    + _L("' longer than 128 chars.") );
    ASSERT( shaderProfile.length()  < 128, _L("Shader profile name '") + shaderProfile + _L("' longer than 128 chars.") );
    size_t count;
    wcstombs_s( &count, pShaderMainAsChar,    shaderMain.c_str(),    128 );
    wcstombs_s( &count, pShaderProfileAsChar, shaderProfile.c_str(), 128 );

    // use DirectX to compile the shader file
    ID3DBlob *pErrorBlob = NULL;
    D3D10_SHADER_MACRO pShaderMacros[2] = { "_CPUT", "1", NULL, NULL }; // TODO: Support passed-in, and defined in .mtl file.  Perhaps under [Shader Defines], etc
    char *pShaderMainAsChars = ws2s(shaderMain.c_str());
    HRESULT hr = D3DCompile(
        pShaderSource,     // shader as a string
        strlen( pShaderSource ), //
        pShaderMainAsChars, // Use entrypoint as file name
        pShaderMacros,        // macro define's
        D3D_COMPILE_STANDARD_FILE_INCLUDE,                 // includes
        pShaderMainAsChar,    // main function name
        pShaderProfileAsChar, // shader profile/feature level
        0,                    // flags 1
        0,                    // flags 2
        ppBlob,               // blob data with compiled code
        &pErrorBlob           // any compile errors stored here
    );
    ASSERT( SUCCEEDED(hr), _L("Error compiling shader '") + shaderMain + _L("'.\n") + (pErrorBlob ? s2ws((char*)pErrorBlob->GetBufferPointer()) : _L("no error message") ) );
    if(pErrorBlob)
    {
        pErrorBlob->Release();
    }
    delete pShaderMainAsChars;
    return result;
}
