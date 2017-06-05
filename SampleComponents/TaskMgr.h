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
