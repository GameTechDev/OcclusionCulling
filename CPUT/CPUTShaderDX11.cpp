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

#include "CPUTShaderDX11.h"
#include "D3DCompiler.h"
#include "CPUTConfigBlock.h"
#include "CPUTMaterial.h"

//-----------------------------------------------------------------------------
bool CPUTShaderDX11::ShaderRequiresPerModelPayload( CPUTConfigBlock &properties )
{
    ID3D11ShaderReflection *pReflector = NULL;

    D3DReflect( mpBlob->GetBufferPointer(), mpBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);
    // Walk through the shader input bind descriptors.
    // If any of them begin with '@', then we need a unique material per model (i.e., we need to clone the material).
    int ii=0;
    D3D11_SHADER_INPUT_BIND_DESC desc;
    HRESULT hr = pReflector->GetResourceBindingDesc( ii++, &desc );
    while( SUCCEEDED(hr) )
    {
        cString tagName = s2ws(desc.Name);
        CPUTConfigEntry *pValue = properties.GetValueByName(tagName);
        if( !pValue->IsValid() )
        {
            // We didn't find our property in the file.  Is it in the global config block?
            pValue = CPUTMaterial::mGlobalProperties.GetValueByName(tagName);
        }
        cString boundName = pValue->ValueAsString();
        if( (boundName.length() > 0) && ((boundName[0] == '@') || (boundName[0] == '#')) )
        {
            return true;
        }
        hr = pReflector->GetResourceBindingDesc( ii++, &desc );
    }
    return false;
}
