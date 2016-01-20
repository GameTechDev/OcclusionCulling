/*!
    \file TaskMgrSS.h

    TaksMgrSS is a class that uses a custom Windows threads schedulers with a
    C-style handle and callback mechanism for scheduling tasks across any 
    number of CPU cores.

    TaskMgrSS is a singleton object and is already instantiated for the app as
    gTaskMgrSS.  The TaskMgrSS is single threaded, meaning that tasksets can only
    be created from one thread. With minor changes and some performance loss,
    multiple threads can create tsk sets (see AllocateTaskSet in TaskMgrSS.cpp).
    The app can control two knobs in the TaskMgrSS class through MAX_SUCCESSORS
    and MAX_TASKSETS degined below.

    MAX_SUCCESSORS is the maximum number of tasksets that can be have depended
    on another taskset.  For example if you have Tasksets A,B,C and both B and 
    C can run simultaniously and both depend on A to complete (so A->(B,C)) then
    A has two successors.  There is a small performance hit for large numbers
    of successors so the value should be set to something reasonably close to 
    the maximum the app will use. The default value is 5.

    MAX_TASKSETS is the max number of tasksets that can be live at one  time. 
    A taskset is live if it has a non-zero reference count.  Increasing the 
    number of tasksets that can be live increases the memory footprint of the 
    TaskMgrSS object.  The default value is 255.

    Copyright 2011 Intel Corporation
    All Rights Reserved

    Permission is granted to use, copy, distribute and prepare derivative works of this
    software for any purpose and without fee, provided, that the above copyright notice
    and this statement appear in all copies.  Intel makes no representations about the
    suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED ""AS IS.""
    INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
    INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
    INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
    assume any responsibility for any errors which may appear in this software nor any
    responsibility to update it.
*/
#pragma once

#include <wtypes.h>
#include "Profile.h"
#include "TaskMgrCommon.h"

  // DYAMIC_BASE is used when the SampleComponents have dynamic
  // switching between schedulers enabled. When either using this
  // as a stand alone module or with a static scheduler is does
  // nothing.
#ifndef  DYNAMIC_BASE
#   define DYNAMIC_BASE
#endif
#define CACHE_ALIGN __declspec(align(64))

#include "spin_mutex.h"
#include "TaskScheduler.h"

/*! The TaskMgrSS allows the user to schedule tasksets All TaskMgrSS 
    functions are NOT threadsafe.  TaskMgr is designed to be called 
    only from the main thread.  Multi-threading is achieved by 
    creating TaskSets that execte on threads created by a Windows
    threads based scheduler.
*/
class TaskMgrSS DYNAMIC_BASE
{
public:
    TaskMgrSS();
    ~TaskMgrSS();

    //  Init will setup the tasking system.  It must be called before
    //  any other functions on the TaskMgrSS interface.
    BOOL
        Init();

    //  Shutdown will stop the tasking system. Any outstanding tasks will
    //  be terminated and the threads used by SS will be released.  It is
    //  up to the application to wait on any outstanding tasks before it
    //  calls shutdown.
    VOID
        Shutdown();

    //  Creates a task set and provides a handle to allow the application
    //  CreateTaskSet can fail if, by adding this task to the successor lists
    //  of its dependecies the list exceeds MAX_SUCCESSORS.  To fix, increase
    //  MAX_SUCCESSORS.
    //
    //  NOTE: A tasket of size 1 is valid.  The most common case is to have 
    //  tasksets of >> 1 so the default tasking primitive is a taskset rather
    //  than a task.
    BOOL  CreateTaskSet(TASKSETFUNC                 pFunc,        //  Function pointer to the 
                                                                  //  Taskset callback function
                        VOID*                       pArg,         //  App data pointer (can be NULL)
                        UINT                        uTaskCount,   //  Number of tasks to create 
                        TASKSETHANDLE*              pDepends,     //  Array of TASKSETHANDLEs that 
                                                                  //  this taskset depends on.  The 
                                                                  //  taskset will not be scheduled
                                                                  //  until all tasksets in this list
                                                                  //  complete.
                        UINT                        uDepends,     //  Count of the depends list
                        OPTIONAL LPCSTR             szSetName,    //  [Optional] name of the taskset
                                                                  //  the name is used for profiling
                        OUT TASKSETHANDLE*          pOutHandle);  //  [Out] Handle to the new taskset

    //  All TASKSETHANDLE must be released when no longer referenced.  
    //  ReleaseHandle will release the Applications reference on the taskset.
    //  It should only be called once per handle returned from CreateTaskSet.
    VOID ReleaseHandle( TASKSETHANDLE hSet );        //  Taskset handle to release

    //  All TASKSETHANDLE must be released when no longer referenced.  
    //  ReleaseHandles will release the Applications reference on the array
    //  of taskset handled specified.  It should only be called once per handle 
    //  returned from CreateTaskSet.
    VOID ReleaseHandles( TASKSETHANDLE* phSet,  //  Taskset handle array to release
                         UINT uSet );           //  count of taskset handle array

    //  WaitForSet will yeild the main thread to the tasking system and return
    //  only when the taskset specified has completed execution.
    VOID WaitForSet( TASKSETHANDLE hSet );      // Taskset to wait for completion

    //  IsSetComplete simple checks to see if the given taskset has completed. It
    //  does not block.
    BOOL IsSetComplete( TASKSETHANDLE hSet );    // Taskset to check completion of

    //  DEMO ONLY: set variable before calling init to the
    //  number of threads SS should create.  Changing this value will
    //  result in inaccurate performance timings.
    //
    //  TaskScheduler should be allowed to use all available cores.  For samples, only
    //  the main thread does work.  In a real application where other 
    //  systems occupy a set of cores, SS thread count should be reduced by
    //  the number of fully utilized cores.
    INT miDemoModeThreadCountOverride;
private:

    class TaskSet
    {
    public:
        TaskSet();

          // Executes a single task on a thread identified by iContextId
        void Execute(INT iContextId);

          // Marks the TaskSetSS as completed
        void CompleteTaskSet();

          // Data and callback for the Task to execute
        TASKSETFUNC             mpFunc;
        void*                   mpvArg;

          // Interal bookkeeping for for managing the TaskSet
        BOOL                       mbCompleted;
        volatile UINT              muRefCount;
        UINT                       muSize;  

          // Lock to keep threads from destroying the successor list
        spin_mutex    mSuccessorsLock;

          // 
        TASKSETHANDLE mhTaskset;
        volatile UINT muStartCount;
        TaskSet*      Successors[ MAX_SUCCESSORS ];
        CHAR          mszSetName[ MAX_TASKSETNAMELENGTH ];

        volatile long  muCompletionCount;
        volatile long  muTaskId;
    };

    friend class TaskScheduler;
    friend class TaskSetSS;

    //  INTERNAL:
    //  Allocate a free slot in the mSets list
    TASKSETHANDLE AllocateTaskSet();

    //  INTERNAL:
    //  Called by the tasking system when a task in a set completes.
    VOID CompleteTaskSet( TASKSETHANDLE hSet );

    VOID ExecuteTask( TASKSETHANDLE hSet );


    //  Array containing the SS task parents.
    TaskSet mSets[ MAX_TASKSETS ];

    //  Helper array index of next free task slot.
    UINT muNextFreeSet;

    //  Pointer to the task scheduler
    TaskScheduler mTaskScheduler;

};

//
//  Forward decl of the TaskMgrSS instance defined in TaskMgrSS.cpp
//
extern TaskMgrSS   gTaskMgrSS;
