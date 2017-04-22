/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPUTBUFFERDX11_H
#define _CPUTBUFFERDX11_H

#include "CPUTBuffer.h"
#include "CPUT_DX11.h"

//--------------------------------------------------------------------------------------
// TODO: Move to dedicated file
class CPUTBufferDX11 : public CPUTBuffer
{
private:
    // resource view pointer
    ID3D11ShaderResourceView  *mpShaderResourceView;
    ID3D11UnorderedAccessView *mpUnorderedAccessView;
    ID3D11Buffer              *mpBuffer;
    ID3D11Buffer              *mpBufferStaging;

    // Destructor is not public.  Must release instead of delete.
    ~CPUTBufferDX11() {
        SAFE_RELEASE( mpShaderResourceView );
        SAFE_RELEASE( mpUnorderedAccessView );
        SAFE_RELEASE( mpBuffer );
        SAFE_RELEASE( mpBufferStaging );
    }

public:
    CPUTBufferDX11() :
        mpShaderResourceView(NULL),
        mpUnorderedAccessView(NULL),
        mpBuffer(NULL),
        mpBufferStaging(NULL)
    {
    }
    CPUTBufferDX11(cString &name, ID3D11Buffer *pBuffer) :
        mpBuffer(pBuffer),
        mpBufferStaging(NULL),
        mpShaderResourceView(NULL),
        mpUnorderedAccessView(NULL),
        CPUTBuffer(name)
    {
        if(pBuffer) pBuffer->AddRef();
    }

    CPUTBufferDX11(cString &name, ID3D11Buffer *pBuffer, ID3D11ShaderResourceView *pView) :
        mpBuffer(pBuffer),
        mpBufferStaging(NULL),
        mpShaderResourceView(pView),
        mpUnorderedAccessView(NULL),
        CPUTBuffer(name)
    {
        if(pBuffer) pBuffer->AddRef();
        if(pView) pView->AddRef();
    }

    CPUTBufferDX11(cString &name, ID3D11Buffer *pBuffer, ID3D11UnorderedAccessView *pView) :
        mpBuffer(pBuffer),
        mpBufferStaging(NULL),
        mpShaderResourceView(NULL),
        mpUnorderedAccessView(pView),
        CPUTBuffer(name)
    {
        if(pBuffer) pBuffer->AddRef();
        if(pView) pView->AddRef();
    }

    ID3D11ShaderResourceView *GetShaderResourceView()
    {
        return mpShaderResourceView;
    }

    ID3D11UnorderedAccessView *GetUnorderedAccessView()
    {
        return mpUnorderedAccessView;
    }

    void SetShaderResourceView(ID3D11ShaderResourceView *pShaderResourceView)
    {
        // release any resource view we might already be pointing too
        SAFE_RELEASE( mpShaderResourceView );
        mpShaderResourceView = pShaderResourceView;
        mpShaderResourceView->AddRef();
    }
    void SetUnorderedAccessView(ID3D11UnorderedAccessView *pUnorderedAccessView)
    {
        // release any resource view we might already be pointing too
        SAFE_RELEASE( mpUnorderedAccessView );
        mpUnorderedAccessView = pUnorderedAccessView;
        mpUnorderedAccessView->AddRef();
    }
    void SetBufferAndViews(ID3D11Buffer *pBuffer, ID3D11ShaderResourceView *pShaderResourceView, ID3D11UnorderedAccessView *pUnorderedAccessView )
    {
        SAFE_RELEASE(mpBuffer);
        mpBuffer = pBuffer;
        if(mpBuffer) mpBuffer->AddRef();

        // release any resource view we might already be pointing too
        SAFE_RELEASE( mpShaderResourceView );
        mpShaderResourceView = pShaderResourceView;
        if(mpShaderResourceView) mpShaderResourceView->AddRef();

        // release any resource view we might already be pointing too
        SAFE_RELEASE( mpUnorderedAccessView );
        mpUnorderedAccessView = pUnorderedAccessView;
        if(mpUnorderedAccessView) mpUnorderedAccessView->AddRef();
    }
    ID3D11Buffer *GetNativeBuffer() { return mpBuffer; }
    D3D11_MAPPED_SUBRESOURCE  MapBuffer(   CPUTRenderParameters &params, eCPUTMapType type, bool wait=true );
    void                      UnmapBuffer( CPUTRenderParameters &params );
    void ReleaseBuffer()
    {
        SAFE_RELEASE(mpShaderResourceView);
        SAFE_RELEASE(mpUnorderedAccessView);
        SAFE_RELEASE(mpBuffer);
        SAFE_RELEASE(mpBufferStaging);
    }
};
#endif //_CPUTBUFFERDX11_H

