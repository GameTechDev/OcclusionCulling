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

#include "CPUT_DX11.h"
#include "CPUTPostProcess.h"
#include "CPUTRenderTarget.h"
#include "CPUTAssetLibrary.h"
#include "CPUTMaterial.h"
#include "CPUTSprite.h"

//-----------------------------------------
CPUTPostProcess::~CPUTPostProcess() {
    SAFE_DELETE( mpFullScreenSprite );

    SAFE_RELEASE( mpMaterialComposite );
    SAFE_RELEASE( mpMaterialBlurVertical );
    SAFE_RELEASE( mpMaterialBlurHorizontal );
    SAFE_RELEASE( mpMaterialDownSampleLogLum );
    SAFE_RELEASE( mpMaterialDownSample4x4Alpha );
    SAFE_RELEASE( mpMaterialDownSample4x4 );
    SAFE_RELEASE( mpMaterialDownSampleBackBuffer4x4 );
    SAFE_RELEASE( mpMaterialSpriteNoAlpha );

    SAFE_DELETE(mpRT1x1 );
    SAFE_DELETE(mpRT4x4 );
    SAFE_DELETE(mpRT64x64 );
    SAFE_DELETE(mpRTDownSample4x4PingPong );
    SAFE_DELETE(mpRTDownSample4x4 );
    // SAFE_DELETE(mpRTSourceRenderTarget ); // We don't allocate this.  Don't delete it.
}

//-----------------------------------------
void CPUTPostProcess::CreatePostProcess(
    CPUTRenderTargetColor *pSourceRenderTarget
){
    mpRTSourceRenderTarget    = pSourceRenderTarget;

    DXGI_FORMAT sourceFormat  = mpRTSourceRenderTarget->GetColorFormat();
    UINT sourceWidth          = mpRTSourceRenderTarget->GetWidth();
    UINT sourceHeight         = mpRTSourceRenderTarget->GetHeight();

    mpRTDownSample4x4         = new CPUTRenderTargetColor();
    mpRTDownSample4x4PingPong = new CPUTRenderTargetColor();
    mpRT64x64                 = new CPUTRenderTargetColor();
    mpRT4x4                   = new CPUTRenderTargetColor();
    mpRT1x1                   = new CPUTRenderTargetColor();

    mpRTDownSample4x4->CreateRenderTarget(         _L("$PostProcessDownsample4x4"),         sourceWidth/4, sourceHeight/4,          sourceFormat );
    mpRTDownSample4x4PingPong->CreateRenderTarget( _L("$PostProcessDownsample4x4PingPong"), sourceWidth/4, sourceHeight/4,          sourceFormat );
    mpRT64x64->CreateRenderTarget(                 _L("$PostProcessRT64x64"),                          64,             64, DXGI_FORMAT_R32_FLOAT ); 
    mpRT4x4->CreateRenderTarget(                   _L("$PostProcessRT4x4"),                             8,              8, DXGI_FORMAT_R32_FLOAT ); 
    mpRT1x1->CreateRenderTarget(                   _L("$PostProcessRT1x1"),                             1,              1, DXGI_FORMAT_R32_FLOAT );

    CPUTAssetLibrary *pLibrary = CPUTAssetLibrary::GetAssetLibrary();
    mpMaterialDownSampleBackBuffer4x4 = pLibrary->GetMaterial(_L("PostProcess/DownSampleBackBuffer4x4"));
    mpMaterialDownSample4x4           = pLibrary->GetMaterial(_L("PostProcess/DownSample4x4"));
    mpMaterialDownSample4x4Alpha      = pLibrary->GetMaterial(_L("PostProcess/DownSample4x4Alpha"));
    mpMaterialDownSampleLogLum        = pLibrary->GetMaterial(_L("PostProcess/DownSampleLogLum"));
    mpMaterialBlurHorizontal          = pLibrary->GetMaterial(_L("PostProcess/BlurHorizontal"));
    mpMaterialBlurVertical            = pLibrary->GetMaterial(_L("PostProcess/BlurVertical"));
    mpMaterialComposite               = pLibrary->GetMaterial(_L("PostProcess/Composite"));
    mpMaterialSpriteNoAlpha           = pLibrary->GetMaterial(_L("PostProcess/Sprite"));

    mpFullScreenSprite = new CPUTSprite();
    mpFullScreenSprite->CreateSprite( -1.0f, -1.0f, 2.0f, 2.0f, _L("Sprite") );
}

UINT gPostProcessingMode = 0;
//-----------------------------------------
void CPUTPostProcess::PerformPostProcess( CPUTRenderParameters &renderParams )
{
    mpRTDownSample4x4->SetRenderTarget( renderParams);
    mpFullScreenSprite->DrawSprite( renderParams, *mpMaterialDownSampleBackBuffer4x4 );
    mpRTDownSample4x4->RestoreRenderTarget( renderParams );

    // Compute average of log of luminance by downsampling log to 64x64, then 4x4, then 1x1
    mpRT64x64->SetRenderTarget(renderParams);
    mpFullScreenSprite->DrawSprite(renderParams, *mpMaterialDownSampleLogLum);
    mpRT64x64->RestoreRenderTarget( renderParams );

    mpRT4x4->SetRenderTarget(renderParams);
    mpFullScreenSprite->DrawSprite(renderParams, *mpMaterialDownSample4x4);
    mpRT4x4->RestoreRenderTarget( renderParams );

    mpRT1x1->SetRenderTarget(renderParams);
    mpFullScreenSprite->DrawSprite( renderParams, *mpMaterialDownSample4x4Alpha ); // Partially blend with previous to smooth result over time
    mpRT1x1->RestoreRenderTarget( renderParams );

    // Better blur for bloom
    UINT ii;
    UINT numBlurs = 1; // TODO: expose as a config param
    for( ii=0; ii<numBlurs; ii++ )
    {
        mpRTDownSample4x4PingPong->SetRenderTarget(renderParams);
        mpFullScreenSprite->DrawSprite( renderParams, *mpMaterialBlurHorizontal );
        mpRTDownSample4x4PingPong->RestoreRenderTarget( renderParams );

        mpRTDownSample4x4->SetRenderTarget( renderParams);
        mpFullScreenSprite->DrawSprite( renderParams, *mpMaterialBlurVertical );
        mpRTDownSample4x4->RestoreRenderTarget( renderParams );
    }
    mpFullScreenSprite->DrawSprite(renderParams, *mpMaterialComposite);
}
