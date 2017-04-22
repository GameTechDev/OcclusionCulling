/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPUTPOSTPROCESS_H
#define _CPUTPOSTPROCESS_H

class CPUTRenderTargetColor;
class CPUTMaterial;
class CPUTRenderParameters;
class CPUTSprite;

class CPUTPostProcess
{
protected:
    CPUTRenderTargetColor *mpRTSourceRenderTarget;
    CPUTRenderTargetColor *mpRTDownSample4x4;
    CPUTRenderTargetColor *mpRTDownSample4x4PingPong;
    CPUTRenderTargetColor *mpRT64x64;
    CPUTRenderTargetColor *mpRT4x4;
    CPUTRenderTargetColor *mpRT1x1;

    CPUTMaterial *mpMaterialSpriteNoAlpha;
    CPUTMaterial *mpMaterialDownSampleBackBuffer4x4;
    CPUTMaterial *mpMaterialDownSample4x4;
    CPUTMaterial *mpMaterialDownSample4x4Alpha;
    CPUTMaterial *mpMaterialDownSampleLogLum;
    CPUTMaterial *mpMaterialBlurHorizontal;
    CPUTMaterial *mpMaterialBlurVertical;
    CPUTMaterial *mpMaterialComposite;

    CPUTSprite   *mpFullScreenSprite;

public:
    CPUTPostProcess() :
        mpRTSourceRenderTarget(NULL),
        mpRTDownSample4x4(NULL),
        mpRTDownSample4x4PingPong(NULL),
        mpRT64x64(NULL),
        mpRT4x4(NULL),
        mpRT1x1(NULL),
        mpMaterialSpriteNoAlpha(NULL),
        mpMaterialDownSampleBackBuffer4x4(NULL),
        mpMaterialDownSample4x4(NULL),
        mpMaterialDownSample4x4Alpha(NULL),
        mpMaterialDownSampleLogLum(NULL),
        mpMaterialBlurHorizontal(NULL),
        mpMaterialBlurVertical(NULL),
        mpMaterialComposite(NULL),
        mpFullScreenSprite(NULL)
    {}
    ~CPUTPostProcess();

    void CreatePostProcess( CPUTRenderTargetColor *pSourceRenderTarget );
    void PerformPostProcess(CPUTRenderParameters &renderParams);
};

#endif // _CPUTPOSTPROCESS_H
