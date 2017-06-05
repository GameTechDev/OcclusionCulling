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
#ifndef _CPUTFRUSTUM_H
#define _CPUTFRUSTUM_H

#include "CPUT.h"
#include "CPUTMath.h"

class CPUTCamera;

class CPUTFrustum
{
public:
    float3 mpPosition[8];
    float3 mpNormal[6];

	float *mPlanes;

    UINT mNumFrustumVisibleModels;
    UINT mNumFrustumCulledModels;

    CPUTFrustum();
    ~CPUTFrustum();

    void InitializeFrustum
    (
        float nearClipDistance,
        float farClipDistance,
        float aspectRatio,
        float fov,
        const float3 &position,
        const float3 &look,
        const float3 &up
    );

    void InitializeFrustum( CPUTCamera *pCamera );

    bool IsVisible(
        const float3 &center,
        const float3 &half
    );

};

#endif // _CPUTFRUSTUM_H
