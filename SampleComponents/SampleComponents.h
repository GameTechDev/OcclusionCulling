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

#ifndef __SAMPLECOMPONENTS_H
#define __SAMPLECOMPONENTS_H

  // Controls for determining which scheduler to use. All option can be configured
  // without changing code relying on the TaskMgrAPI.

  // Dynamic allows run-time switching of scheduler (Default)
//#define DYNAMIC_SCHEDULER
  // Uses Threading Building Blocks as the only scheduler
#define STATIC_TBB
  // Uses Concurrency Runtime as the only scheduler
//#define STATIC_CRT
  // Uses Simple Scheduler (custom) as the only scheduler
//#define STATIC_SS
#include "TaskMgr.h"


#endif //__SAMPLECONPONENTS_H
