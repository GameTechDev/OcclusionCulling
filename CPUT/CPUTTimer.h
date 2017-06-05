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
#ifndef CPUTTIMER_H
#define CPUTTIMER_H

class CPUTTimer
{
public:
	virtual void   StartTimer()     = 0;
    virtual double StopTimer()      = 0;
	virtual double GetTotalTime()   = 0; // In seconds
	virtual double GetElapsedTime() = 0; // In seconds
    virtual void   ResetTimer()     = 0;
};

#endif CPUTTIMER_H