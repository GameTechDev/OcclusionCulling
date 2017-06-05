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
#ifndef _CPUTTEXTUREDX11_H
#define _CPUTTEXTUREDX11_H

#include "CPUTTexture.h"
#include "CPUT_DX11.h"
#include <d3d11.h>

class CPUTTextureDX11 : public CPUTTexture
{
private:
    // resource view pointer
    CD3D11_TEXTURE2D_DESC     mDesc;
    ID3D11ShaderResourceView *mpShaderResourceView;
    ID3D11Resource           *mpTexture;
    ID3D11Resource           *mpTextureStaging;

    // Destructor is not public.  Must release instead of delete.
    ~CPUTTextureDX11() {
        SAFE_RELEASE( mpShaderResourceView );
        SAFE_RELEASE( mpTexture );
        SAFE_RELEASE( mpTextureStaging );
    }

public:
    static const cString &GetDXGIFormatString(DXGI_FORMAT Format);
    static CPUTResult     GetSRGBEquivalent(DXGI_FORMAT inFormat, DXGI_FORMAT& sRGBFormat);
    static bool           DoesExistEquivalentSRGBFormat(DXGI_FORMAT inFormat);
    static CPUTTexture   *CreateTexture( const cString &name, const cString &absolutePathAndFilename, bool loadAsSRGB );
    static CPUTResult     CreateNativeTexture(
                              ID3D11Device *pD3dDevice,
                              const cString &fileName,
                              ID3D11ShaderResourceView **ppShaderResourceView,
                              ID3D11Resource **ppTexture,
                              bool forceLoadAsSRGB
                          );

    CPUTTextureDX11() :
        mpShaderResourceView(NULL),
        mpTexture(NULL),
        mpTextureStaging(NULL)
    {}
    CPUTTextureDX11(cString &name) :
        mpShaderResourceView(NULL),
        mpTexture(NULL),
        mpTextureStaging(NULL),
        CPUTTexture(name)
    {}
    CPUTTextureDX11(cString &name, ID3D11Resource *pTextureResource, ID3D11ShaderResourceView *pSrv ) :
        mpTextureStaging(NULL),
        CPUTTexture(name)
    {
        mpShaderResourceView = pSrv;
        if(mpShaderResourceView) pSrv->AddRef();
        mpTexture = pTextureResource;
        if(mpTexture) mpTexture->AddRef();
    }

    void ReleaseTexture()
    {
        SAFE_RELEASE(mpShaderResourceView);
        SAFE_RELEASE(mpTexture);
    }
    void SetTexture(ID3D11Resource *pTextureResource, ID3D11ShaderResourceView *pSrv )
    {
        mpShaderResourceView = pSrv;
        if(mpShaderResourceView) pSrv->AddRef();

        mpTexture = pTextureResource;
        if(mpTexture) mpTexture->AddRef();
    }

    ID3D11ShaderResourceView* GetShaderResourceView()
    {
        return mpShaderResourceView;
    }

    void SetTextureAndShaderResourceView(ID3D11Resource *pTexture, ID3D11ShaderResourceView *pShaderResourceView)
    {
        // release any resources we might already be pointing too
        SAFE_RELEASE( mpTexture );
        SAFE_RELEASE( mpTextureStaging ); // Now out-of sync.  Will be recreated on next Map().
        SAFE_RELEASE( mpShaderResourceView );
        mpTexture = pTexture;
        if( mpTexture ) mpTexture->AddRef();
        mpShaderResourceView = pShaderResourceView;
        mpShaderResourceView->AddRef();
    }
    D3D11_MAPPED_SUBRESOURCE  MapTexture(   CPUTRenderParameters &params, eCPUTMapType type, bool wait=true );
    void                      UnmapTexture( CPUTRenderParameters &params );
};

#endif //_CPUTTEXTUREDX11_H

