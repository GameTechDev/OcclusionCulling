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
#include "CPUT.h"
#include "CPUTAssetLibrary.h"

#ifdef CPUT_FOR_DX11
#include "CPUTMaterialDX11.h"
#else    
    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
#endif


CPUTMaterial *CPUTMaterial::CreateMaterial( const cString &absolutePathAndFilename, const CPUTModel *pModel, int meshIndex )
{
    // material was not in the library, so load it
#ifdef CPUT_FOR_DX11
    CPUTMaterial *pMaterial = new CPUTMaterialDX11();
#else    
    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
#endif
    CPUTResult result = pMaterial->LoadMaterial(absolutePathAndFilename, pModel, meshIndex);
    ASSERT( CPUTSUCCESS(result), _L("\nError - CPUTAssetLibrary::GetMaterial() - Error in material file: '")+absolutePathAndFilename+_L("'") );

    // add material to material library list
    if( pModel && pMaterial->MaterialRequiresPerModelPayload() )
    {
        CPUTAssetLibrary::GetAssetLibrary()->AddMaterial( absolutePathAndFilename, pMaterial, pModel, meshIndex );
    } else
    {
        CPUTAssetLibrary::GetAssetLibrary()->AddMaterial( absolutePathAndFilename, pMaterial );
    }

    return pMaterial;
}
