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
#ifndef __CPUTPARSELIBRARY_H__
#define __CPUTPARSELIBRARY_H__



#include "CPUT.h"

#include <algorithm> // for std::transform

#if !defined(UNICODE) && !defined(_UNICODE)
#define fgetws      fgets
#define swscanf_s   sscanf_s
#define wcstok_s    strtok_s
#define wcsncmp     strncmp
#define _wtoi       atoi
#define _wtol       atol
#endif

typedef UINT UINT;

class CPUTConfigEntry
{
private:
    cString szName;
    cString szValue;

    friend class CPUTConfigBlock;
    friend class CPUTConfigFile;

public:
    CPUTConfigEntry() {}
    CPUTConfigEntry(const cString &name, const cString &value): szName(name), szValue(value){};

    static CPUTConfigEntry  &sNullConfigValue;

    const cString & NameAsString(void){ return  szName;};
    const cString & ValueAsString(void){ return szValue; }
	bool IsValid(void){ return !szName.empty(); }
    float ValueAsFloat(void)
    {
        float fValue=0;
        int retVal;
        retVal=swscanf_s(szValue.c_str(), _L("%g"), &fValue ); // float (regular float, or E exponentially notated float)
        ASSERT(0!=retVal, _L("ValueAsFloat - value specified is not a float"));
        return fValue;
    }
    int ValueAsInt(void)
    {
        int nValue=0;
        int retVal;
        retVal=swscanf_s(szValue.c_str(), _L("%d"), &nValue ); // signed int (NON-hex)
        ASSERT(0!=retVal, _L("ValueAsInt - value specified is not a signed int"));
        return nValue;
    }
    UINT ValueAsUint(void)
    {
        UINT nValue=0;
        int retVal;
        retVal=swscanf_s(szValue.c_str(), _L("%u"), &nValue ); // unsigned int
        ASSERT(0!=retVal, _L("ValueAsUint - value specified is not a UINT"));
        return nValue;
    }
    bool ValueAsBool(void)
    {
        return  (szValue.compare(_L("true")) == 0) || 
                (szValue.compare(_L("1")) == 0) || 
                (szValue.compare(_L("t")) == 0);
    }

    void ValueAsFloatArray(float *pFloats, int count);
};

class CPUTConfigBlock
{
public:
    CPUTConfigBlock();
    ~CPUTConfigBlock();

    CPUTConfigEntry *AddValue(const cString &szName, const cString &szValue);
    CPUTConfigEntry *GetValue(int nValueIndex);
    CPUTConfigEntry *GetValueByName(const cString &szName);
    const cString &GetName(void);
    int GetNameValue(void);
    int ValueCount(void);
    bool IsValid() { return mnValueCount > 0; }
private:
    CPUTConfigEntry mpValues[64];
    CPUTConfigEntry mName;
    cString         mszName;
    int             mnValueCount;

    friend class CPUTConfigFile;
};

class CPUTConfigFile
{
public:
    CPUTConfigFile();
    ~CPUTConfigFile();

    CPUTResult LoadFile(const cString &szFilename);

    CPUTConfigBlock *GetBlock(int nBlockIndex);
    CPUTConfigBlock *GetBlockByName(const cString &szBlockName);
    int BlockCount(void);
private:
    CPUTConfigBlock    *mpBlocks;
    int                 mnBlockCount;
};

#endif //#ifndef __CPUTPARSELIBRARY_H__