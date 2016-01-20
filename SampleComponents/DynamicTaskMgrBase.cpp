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