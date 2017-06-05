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
#ifndef __CPUTFONT_H__
#define __CPUTFONT_H__

#include "CPUT.h"
#include "CPUTRefCount.h"

#define CPUT_MAX_NUMBER_OF_CHARACTERS 256

class CPUTFont : public CPUTRefCount
{
friend class CPUTFontDX11;

public:
    static CPUTFont *CreateFont( cString FontName, cString AbsolutePathAndFilename);
    
    CPUT_SIZE GetGlyphSize(const char c);
    void GetGlyphUVS(const char c, const bool bEnabledVersion, float3& UV1, float3& UV2);

protected:
    ~CPUTFont(){}  // Destructor is not public.  Must release instead of delete.

    // atlas texture info
    float mAtlasWidth;
    float mAtlasHeight;
    float mDisabledYOffset;
    UINT mNumberOfGlyphsInAtlas;

    int mpGlyphMap[CPUT_MAX_NUMBER_OF_CHARACTERS];    

    int mpGlyphStarts[CPUT_MAX_NUMBER_OF_CHARACTERS];
    CPUT_SIZE mpGlyphSizes[CPUT_MAX_NUMBER_OF_CHARACTERS];
    float mpGlyphUVCoords[4*CPUT_MAX_NUMBER_OF_CHARACTERS]; // 4 floats/glyph = upper-left:(uv1.x, uv1.y), lower-right:(uv2.x, uv2.y)
    float mpGlyphUVCoordsDisabled[4*CPUT_MAX_NUMBER_OF_CHARACTERS]; // 4 floats/glyph = upper-left:(uv1.x, uv1.y), lower-right:(uv2.x, uv2.y)

    // helper functions
    int FindGlyphIndex(const char c);
};

#endif // #ifndef __CPUTFONT_H__