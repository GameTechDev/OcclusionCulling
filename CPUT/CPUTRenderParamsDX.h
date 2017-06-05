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
#ifndef __CPUTRENDERPARAMSDX_H__
#define __CPUTRENDERPARAMSDX_H__

#include "CPUT.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include "CPUTRenderParams.h"

class CPUTRenderParametersDX : public CPUTRenderParameters
{
public:
    ID3D11DeviceContext *mpContext;

public:
    CPUTRenderParametersDX(): mpContext(NULL){}
    CPUTRenderParametersDX( ID3D11DeviceContext *pContext, bool drawModels=true, bool renderOnlyVisibleModels=true, bool showBoundingBoxes=false )
        : mpContext(pContext)
    {
        mShowBoundingBoxes       = showBoundingBoxes;
        mDrawModels              = drawModels;
        mRenderOnlyVisibleModels = renderOnlyVisibleModels;
    }
    ~CPUTRenderParametersDX(){}
};

#endif // #ifndef __CPUTRENDERPARAMSDX_H__