//--------------------------------------------------------------------------------------
// Copyright 2010 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

//---------------------------------------------------------------------------------------

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
