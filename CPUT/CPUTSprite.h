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
#ifndef _CPUTSPRITE_H
#define _CPUTSPRITE_H

#include "CPUT.h"
#include "d3d11.h"

class CPUTMaterial;
class CPUTRenderParameters;
class CPUTTexture;

class CPUTSprite
{
protected:
    class SpriteVertex
    {
    public:
        float mpPos[3];
        float mpUV[2];
    };

    ID3D11Buffer      *mpVertexBuffer;
    CPUTMaterial      *mpMaterial;
    ID3D11InputLayout *mpInputLayout;

public:
    CPUTSprite() :
        mpInputLayout(NULL),
        mpVertexBuffer(NULL),
        mpMaterial(NULL)
    {
    }
    ~CPUTSprite();
    HRESULT CreateSprite(
        float          spriteX = -1.0f,
        float          spriteY = -1.0f,
        float          spriteWidth  = 2.0f,
        float          spriteHeight = 2.0f,
        const cString &materialName = cString(_L("Sprite"))
    );
    void DrawSprite( CPUTRenderParameters &renderParams ) { DrawSprite( renderParams, *mpMaterial ); }
    void DrawSprite( CPUTRenderParameters &renderParams, CPUTMaterial &material );
};

#endif // _CPUTSPRITE_H
