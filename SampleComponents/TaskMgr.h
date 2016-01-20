//--------------------------------------------------------------------------------------
// 
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
#pragma once

struct TaskMgrID
{
    enum TaskManagerIDs
    {
    #if defined(DYNAMIC_SCHEDULER)
        TBB,

    #if _MSC_VER >= 1600 
        CRT,
    #endif

        SS,

    #elif defined(STATIC_TBB)
        TBB,
    #elif defined(STATIC_SS)
        SS,
    #elif defined(STATIC_CRT) && _MSC_VER >= 1600 
        CRT,
    #endif

        Count,
    };
};

#ifdef DYNAMIC_SCHEDULER
    #include "DynamicTaskMgrBase.h"
    #define DYNAMIC_BASE : public DynamicTaskMgrBase

    #include "TaskMgrSS.h"
    #include "TaskMgrTbb.h"

    #if _MSC_VER >= 1600
        #include "TaskMgrCRT.h"
    #endif
    extern DynamicTaskMgrBase* g_pTaskMgr;
#else
    #define DYNAMIC_BASE
    #include "TaskMgrSS.h"
    #include "TaskMgrTbb.h"
    #if _MSC_VER >= 1600
        #include "TaskMgrCRT.h"
    #endif
    #if defined(STATIC_SS)
        extern TaskMgrSS* g_pTaskMgr;
    #elif defined(STATIC_TBB)
        extern TaskMgrTbb* g_pTaskMgr;
    #elif defined(STATIC_CRT)
        #if _MSC_VER >= 1600
            extern TaskMgrCRT* g_pTaskMgr;
        #else
            #error "Concurrency Runtime not supported by this compiler"
        #endif
    #endif
#endif

extern const wchar_t *TaskMgrNames[TaskMgrID::Count + 1];
void SetTaskManager(TaskMgrID::TaskManagerIDs id);
