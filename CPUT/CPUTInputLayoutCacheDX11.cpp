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
#include "CPUTInputLayoutCacheDX11.h"
#include "CPUTVertexShaderDX11.h"

extern const cString *gpDXGIFormatNames;

CPUTInputLayoutCacheDX11* CPUTInputLayoutCacheDX11::mpInputLayoutCache = NULL;

//-----------------------------------------------------------------------------
void CPUTInputLayoutCacheDX11::ClearLayoutCache()
{
    // iterate over the entire map - and release each layout object
    std::map<LayoutKey, ID3D11InputLayout*>::iterator mapIterator;

    for(mapIterator = mLayoutList.begin(); mapIterator != mLayoutList.end(); mapIterator++)
    {
        mapIterator->second->Release();  // release the ID3D11InputLayout*
    }
    mLayoutList.clear();
}

// singleton retriever
//-----------------------------------------------------------------------------
CPUTInputLayoutCacheDX11* CPUTInputLayoutCacheDX11::GetInputLayoutCache()
{
    if(NULL == mpInputLayoutCache)
    {
        mpInputLayoutCache = new CPUTInputLayoutCacheDX11();
    }
    return mpInputLayoutCache;
}

// singleton destroy routine
//-----------------------------------------------------------------------------
CPUTResult CPUTInputLayoutCacheDX11::DeleteInputLayoutCache()
{
    if(mpInputLayoutCache)
    {
        delete mpInputLayoutCache;
        mpInputLayoutCache = NULL;
    }
    return CPUT_SUCCESS;
}

// find existing, or create new, ID3D11InputLayout layout
//-----------------------------------------------------------------------------
CPUTResult CPUTInputLayoutCacheDX11::GetLayout(
    ID3D11Device *pDevice,
    D3D11_INPUT_ELEMENT_DESC *pDXLayout,
    CPUTVertexShaderDX11 *pVertexShader,
    ID3D11InputLayout **ppInputLayout
){
    // Generate the vertex layout key
    LayoutKey layoutKey(pDXLayout, pVertexShader, true);

    // Do we already have one like this?
    if( mLayoutList[layoutKey] )
    {
        *ppInputLayout = mLayoutList[layoutKey];
        (*ppInputLayout)->AddRef();
        return CPUT_SUCCESS;
    }
    // Not found, create a new ID3D11InputLayout object

    // How many elements are in the input layout?
    int numInputLayoutElements=0;
    while(NULL != pDXLayout[numInputLayoutElements].SemanticName)
    {
        numInputLayoutElements++;
    }
    // Create the input layout
    HRESULT hr;
    ID3DBlob *pBlob = pVertexShader->GetBlob();
    hr = pDevice->CreateInputLayout( pDXLayout, numInputLayoutElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), ppInputLayout );
    ASSERT( SUCCEEDED(hr), _L("Error creating input layout.") );
    CPUTSetDebugName( *ppInputLayout, _L("CPUTInputLayoutCacheDX11::GetLayout()") );

    // Store this layout object in our map
    mLayoutList[layoutKey] = *ppInputLayout;

    // Addref for storing it in our map as well as returning it (count should be = 2 at this point)
    (*ppInputLayout)->AddRef();

    return CPUT_SUCCESS;
}

//-----------------------------------------------------------------------------
CPUTInputLayoutCacheDX11::LayoutKey::LayoutKey()
    : layout(NULL), vs(NULL), nElems(0), layout_owned(false)
{
}

CPUTInputLayoutCacheDX11::LayoutKey::LayoutKey(const D3D11_INPUT_ELEMENT_DESC *pDXLayout, void *vs, bool just_ref)
{
    nElems = 0;
    while (pDXLayout[nElems].SemanticName)
    {
        ++nElems;
    }

    if (just_ref)
    {
        layout = pDXLayout;
        this->vs = vs;
        layout_owned = false;
    }
    else
    {
        D3D11_INPUT_ELEMENT_DESC *copy = new D3D11_INPUT_ELEMENT_DESC[nElems];
        memcpy(copy, pDXLayout, nElems * sizeof(*copy));
        layout = copy;
        this->vs = vs;
        layout_owned = true;
    }
}

CPUTInputLayoutCacheDX11::LayoutKey::LayoutKey(const LayoutKey &x)
{
    D3D11_INPUT_ELEMENT_DESC *copy = new D3D11_INPUT_ELEMENT_DESC[x.nElems];
    memcpy(copy, x.layout, x.nElems * sizeof(*copy));
    layout = copy;
    vs = x.vs;
    nElems = x.nElems;
    layout_owned = true;
}

CPUTInputLayoutCacheDX11::LayoutKey::~LayoutKey()
{
    if (layout_owned)
    {
        SAFE_DELETE_ARRAY(layout);
    }
}

CPUTInputLayoutCacheDX11::LayoutKey &CPUTInputLayoutCacheDX11::LayoutKey::operator =(const LayoutKey &x)
{
    LayoutKey copy(x);
    std::swap(layout, copy.layout);
    std::swap(vs, copy.vs);
    std::swap(nElems, copy.nElems);
    std::swap(layout_owned, copy.layout_owned);
    return *this;
}