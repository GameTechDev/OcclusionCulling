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
#include "CPUTLight.h"

// Read light properties from .set file
//-----------------------------------------------------------------------------
CPUTResult CPUTLight::LoadLight(CPUTConfigBlock *pBlock, int *pParentID)
{
    ASSERT( (NULL!=pBlock), _L("Invalid NULL parameter.") );

    CPUTResult result = CPUT_SUCCESS;

    // set the null/group node name
    mName = pBlock->GetValueByName(_L("name"))->ValueAsString();

    // get the parent ID
    *pParentID = pBlock->GetValueByName(_L("parent"))->ValueAsInt();

    LoadParentMatrixFromParameterBlock( pBlock );

    cString lightType = pBlock->GetValueByName(_L("lighttype"))->ValueAsString();
    if(lightType.compare(_L("spot")) == 0)
    {
        mLightParams.nLightType = CPUT_LIGHT_SPOT;
    }
    else if(lightType.compare(_L("directional")) == 0)
    {
        mLightParams.nLightType = CPUT_LIGHT_DIRECTIONAL;
    }
    else if(lightType.compare(_L("point")) == 0)
    {
        mLightParams.nLightType = CPUT_LIGHT_POINT;
    }
    else
    {
        // ASSERT(0,_L(""));
        // TODO: why doesn't assert work here?
    }

    pBlock->GetValueByName(_L("Color"))->ValueAsFloatArray(mLightParams.pColor, 3);
    mLightParams.fIntensity    = pBlock->GetValueByName(_L("Intensity"))->ValueAsFloat();
    mLightParams.fHotSpot      = pBlock->GetValueByName(_L("HotSpot"))->ValueAsFloat();
    mLightParams.fConeAngle    = pBlock->GetValueByName(_L("ConeAngle"))->ValueAsFloat();
    mLightParams.fDecayStart   = pBlock->GetValueByName(_L("DecayStart"))->ValueAsFloat();
    mLightParams.bEnableFarAttenuation = pBlock->GetValueByName(_L("EnableNearAttenuation"))->ValueAsBool();
    mLightParams.bEnableFarAttenuation = pBlock->GetValueByName(_L("EnableFarAttenuation"))->ValueAsBool();
    mLightParams.fNearAttenuationStart = pBlock->GetValueByName(_L("NearAttenuationStart"))->ValueAsFloat();
    mLightParams.fNearAttenuationEnd   = pBlock->GetValueByName(_L("NearAttenuationEnd"))->ValueAsFloat();
    mLightParams.fFarAttenuationStart  = pBlock->GetValueByName(_L("FarAttenuationStart"))->ValueAsFloat();
    mLightParams.fFarAttenuationEnd    = pBlock->GetValueByName(_L("FarAttenuationEnd"))->ValueAsFloat();

    return result;
}
