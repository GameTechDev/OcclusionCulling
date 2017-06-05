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
#ifndef __CPUTTIMER_H__
#define __CPUTTIMER_H__



#include "CPUT.h"
#include "Windows.h"
#include "CPUTTimer.h"

// Simple QueryPerformanceCounter()-based timer class
//-----------------------------------------------------------------------------
class CPUTTimerWin : CPUTTimer
{
public:
    CPUTTimerWin();
    virtual void   StartTimer();
    virtual double StopTimer();
    virtual double GetTotalTime();
    virtual double GetElapsedTime();
    virtual void   ResetTimer();

	virtual LARGE_INTEGER GetTimer();
	virtual double GetTime(LARGE_INTEGER t1, LARGE_INTEGER t2);

private:
    bool mbCounting;
    LARGE_INTEGER mlStartCount;
    LARGE_INTEGER mlLastMeasured;
    LARGE_INTEGER mlFrequency;
};


#endif // #ifndef __CPUTTIMER_H__