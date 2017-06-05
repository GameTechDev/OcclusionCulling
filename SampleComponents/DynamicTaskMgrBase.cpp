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
#include "SampleComponents.h"

#if defined(DYNAMIC_SCHEDULER)
    DynamicTaskMgrBase* gTaskMgrs[] = { &gTaskMgr,
    #if _MSC_VER >= 1600
                                        &gTaskMgrCRT,
    #endif
                                        &gTaskMgrSS,

                                      };

    const wchar_t *TaskMgrNames[TaskMgrID::Count + 1] = { TEXT("TBB"),
                                            #if _MSC_VER >= 1600
                                            TEXT("ConcRT"),
                                            #endif
                                            TEXT("SS"),
                                            TEXT("None")
    };

    DynamicTaskMgrBase* g_pTaskMgr = gTaskMgrs[0];

    void SetTaskManager(TaskMgrID::TaskManagerIDs id)
    {
        if(id != TaskMgrID::Count)
        {
            g_pTaskMgr->Shutdown();
            g_pTaskMgr = gTaskMgrs[id];
            g_pTaskMgr->Init();
        }
    }
#else
    #if defined(STATIC_SS)
        TaskMgrSS* g_pTaskMgr = &gTaskMgrSS;
        const wchar_t *TaskMgrNames[TaskMgrID::Count + 1] = { TEXT("SS"),
                                                TEXT("None")
        };
    #elif defined(STATIC_TBB)
        TaskMgrTbb* g_pTaskMgr = &gTaskMgr;
        const wchar_t *TaskMgrNames[TaskMgrID::Count + 1] = { TEXT("TBB"),
                                                TEXT("None")
        };
    #elif defined(STATIC_CRT)
        #if _MSC_VER >= 1600
            TaskMgrCRT* g_pTaskMgr = &gTaskMgrCRT;
            const wchar_t *TaskMgrNames[TaskMgrID::Count + 1] = { TEXT("ConcRT"),
                                                    TEXT("None")
            };
        #endif
    #endif
    void SetTaskManager(TaskMgrID::TaskManagerIDs id)
    {
    }
#endif