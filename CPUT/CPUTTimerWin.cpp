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
#include "CPUTTimerWin.h"

//
// Timer is initially not running and set to a zero state.
// Performance counter frequency is obtained.
//
CPUTTimerWin::CPUTTimerWin():mbCounting(false)
{
    ResetTimer();
    QueryPerformanceFrequency(&mlFrequency);
}

//
// Reset timer to zero
//
void CPUTTimerWin::ResetTimer()
{
    mlStartCount.QuadPart   = 0;
    mlLastMeasured.QuadPart = 0;
    mbCounting              = false;
}

//
// Starts timer and resets everything. If timer is already running,
// nothing happens.
//
void CPUTTimerWin::StartTimer()
{
    if(!mbCounting)
    {
        ResetTimer();
        mbCounting = true;
        QueryPerformanceCounter(&mlLastMeasured);
        mlStartCount = mlLastMeasured;
    }
}

//
// If the timer is running, stops the timer and returns the time in seconds since StartTimer() was called.
//
// If the timer is not running, returns the time in seconds between the
// last start/stop pair.
//
double CPUTTimerWin::StopTimer()
{
    if(mbCounting)
    {
        mbCounting = false;
        QueryPerformanceCounter(&mlLastMeasured);
    }

    return (((double)(mlLastMeasured.QuadPart - mlStartCount.QuadPart)) / ((double)(mlFrequency.QuadPart)));
}

LARGE_INTEGER CPUTTimerWin::GetTimer()
{
	QueryPerformanceCounter(&mlLastMeasured);
	return mlLastMeasured;
}

double CPUTTimerWin::GetTime(LARGE_INTEGER t1, LARGE_INTEGER t2)
{
	return ((double(t2.QuadPart - t1.QuadPart)) / ((double)(mlFrequency.QuadPart)));
}

//
// If the timer is running, returns the total time in seconds since StartTimer was called.
//
// If the timer is not running, returns the time in seconds between the
// last start/stop pair.
//
double CPUTTimerWin::GetTotalTime()
{
    LARGE_INTEGER temp;

    if (mbCounting) {
        QueryPerformanceCounter(&temp);
        return ((double)(temp.QuadPart - mlStartCount.QuadPart) / (double)(mlFrequency.QuadPart));
    }

    return ((double)(mlLastMeasured.QuadPart - mlStartCount.QuadPart) / (double)(mlFrequency.QuadPart));
}

//
// If the timer is running, returns the total time in seconds that the timer
// has run since it was last started or elapsed time was read from. Updates last measured time.
//
// If the timer is not running, returns 0.
//
double CPUTTimerWin::GetElapsedTime()
{
    LARGE_INTEGER temp;
    LARGE_INTEGER elapsedTime;
    elapsedTime.QuadPart = 0;

    if (mbCounting) {
        QueryPerformanceCounter(&temp);
        elapsedTime.QuadPart = temp.QuadPart - mlLastMeasured.QuadPart;
        mlLastMeasured = temp;
    }

    return ((double)elapsedTime.QuadPart / (double)mlFrequency.QuadPart);
}
