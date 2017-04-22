/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPUTTEXTURE_H
#define _CPUTTEXTURE_H

#include "CPUT.h"
#include "CPUTRefCount.h"

class CPUTTexture : public CPUTRefCount
{
protected:
    cString       mName;
    eCPUTMapType  mMappedType;

    ~CPUTTexture(){} // Destructor is not public.  Must release instead of delete.

public:
    CPUTTexture()              : mMappedType(CPUT_MAP_UNDEFINED) {}
	CPUTTexture(cString &name) : mMappedType(CPUT_MAP_UNDEFINED), mName(name) {}
    static CPUTTexture *CreateTexture( const cString &name, const cString absolutePathAndFilename, bool loadAsSRGB );
    virtual D3D11_MAPPED_SUBRESOURCE  MapTexture(   CPUTRenderParameters &params, eCPUTMapType type, bool wait=true ) = 0;
    virtual void                      UnmapTexture( CPUTRenderParameters &params ) =0; // TODO: Store params on Map() and don't require here.
};

#endif //_CPUTTEXTURE_H
