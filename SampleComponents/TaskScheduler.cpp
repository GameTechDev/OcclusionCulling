/*!
    \file TaskScheduler.cpp

    TaskScheduler is a class that manages the distribution of task among the worker
    threads and the lifetime of the worker threads.

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

#include <Windows.h>

#include "TaskMgr.h"
#include "TaskScheduler.h"

#include <new>

#pragma warning ( push )
#pragma warning ( disable : 4995 ) // skip deprecated warning on intrinsics.
#include <intrin.h>
#pragma warning ( pop )


  // helper function to get the number of processors on the system
DWORD get_proc_count()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

DWORD WINAPI TaskScheduler::ThreadMain(VOID* thread_instance)
{
    TaskScheduler *pScheduler = reinterpret_cast<TaskScheduler*>(thread_instance);
    pScheduler->ExecuteTasks();

    return 0;
}

// task_scheduler implementation

  // Initializes the Sheduler and creates the worker threads
VOID TaskScheduler::Init(int thread_count)
{   
      // If the scheduler is still running, ignore this
    if(mbAlive == TRUE) return;

    muContextId = 0;
    mbAlive = TRUE;
    miWriter = 0;
    miTaskCount = 0;

      // Set the buffer of active tasks to empty by marking all of the slots as
      // TASKSETHANDLE_INVALID
    memset(mhActiveTaskSets,-1,sizeof(mhActiveTaskSets));

      // Get the number of worker threads that will be available
    if(thread_count == MAX_THREADS)
    {
          // Leave one core for the main thread.
        miThreadCount = get_proc_count() - 1;
    }
    else
    {
        miThreadCount = thread_count;
    }

    if(miThreadCount == 0)
    {
        mpThreadData = 0;
        return;
    }

    mhTaskAvailable = CreateSemaphore(0,0,miThreadCount,0);

      // Create and initialize all of the threads
    mpThreadData = new HANDLE[miThreadCount];
    for(INT uThread = 0; uThread < miThreadCount; ++uThread)
    {
        mpThreadData[uThread] = CreateThread(0,0,TaskScheduler::ThreadMain,this,0,0);
    }
}
 
  // Clean up the worker threads and associated data
VOID TaskScheduler::Shutdown()
{
      // Tell of of the threads to break out of their loops
    mbAlive = FALSE;
      //Wake up a sleeping threads and wait for them to exit
    LONG sleep_count = 0;
    ReleaseSemaphore(mhTaskAvailable,1,&sleep_count);
    ReleaseSemaphore(mhTaskAvailable,miThreadCount - sleep_count,0);
    WaitForMultipleObjects(miThreadCount,mpThreadData,TRUE,INFINITE);

      // Clean up the handles
    for(INT uThread = 0; uThread < miThreadCount; ++uThread)
        CloseHandle(mpThreadData[uThread]);

    delete [] mpThreadData;
	mpThreadData = 0;
	miThreadCount = 0;
}

  // Main loop for the worker threads
VOID TaskScheduler::ExecuteTasks()
{
      // Get the ID for the thread
    const UINT iContextId = _InterlockedIncrement((LONG*)&muContextId);
      // Start reading from the beginning of the work queue
    INT  iReader = 0;

      // Thread keeps recieving and executing tasks until it is terminated
    while(mbAlive == TRUE)
    {
          // Get a Handle from the work queue
        TASKSETHANDLE handle = mhActiveTaskSets[iReader];

          // If there is a TaskSet in the slot execute a task
        if(handle != TASKSETHANDLE_INVALID)
        {
            TaskMgrSS::TaskSet *pSet = &gTaskMgrSS.mSets[handle];
            if(pSet->muCompletionCount > 0 && pSet->muTaskId >= 0)
            {
                pSet->Execute(iContextId);
            }
            else
            {
                _InterlockedCompareExchange((LONG*)&mhActiveTaskSets[iReader],TASKSETHANDLE_INVALID,handle);
                iReader = (iReader + 1) & (MAX_TASKSETS - 1);
            }
        }
          // Otherwise keep looking for work
        else if(miTaskCount > 0)
        {
            iReader = (iReader + 1) & (MAX_TASKSETS - 1);
        }
          // or sleep if all of the work has been completed
        else
        {
            if(miTaskCount <= 0)
            {
                WaitForSingleObject(mhTaskAvailable,INFINITE);
            }
        }
    }
}

  // Adds a task set to the work queue
VOID TaskScheduler::AddTaskSet( TASKSETHANDLE hSet, INT iTaskCount )
{
      // Increase the Task Count before adding the tasks to keep the
      // workers from going to sleep during this process
    _InterlockedExchangeAdd((LONG*)&miTaskCount,iTaskCount);

      // Looks for an open slot starting at the end of the queue
    INT iWriter = miWriter;
    do
    {
        while(mhActiveTaskSets[iWriter] != TASKSETHANDLE_INVALID)
            iWriter = (iWriter + 1) & (MAX_TASKSETS - 1);

        // verify that another thread hasn't already written to this slot
    } while(_InterlockedCompareExchange((LONG*)&mhActiveTaskSets[iWriter],hSet,TASKSETHANDLE_INVALID) != TASKSETHANDLE_INVALID);

      // Wake up all suspended threads
    LONG sleep_count = 0;
    ReleaseSemaphore(mhTaskAvailable,1,&sleep_count);
    INT iCountToWake = iTaskCount < (miThreadCount - sleep_count - 1) ? iTaskCount : miThreadCount - sleep_count - 1;
    ReleaseSemaphore(mhTaskAvailable,iCountToWake,&sleep_count);

      // reset the end of the queue
    miWriter = iWriter;
}

  // Yields the main thread to the scheduler when it needs to wait for a Task Set to be completed
VOID TaskScheduler::WaitForFlag( volatile BOOL *pFlag )
{
      // Start at the the end of the work queue
    int iReader = miWriter;

      // The condition for exiting this loop is changed externally to the function,
      // possibly in another thread.  The loop will break with no more than one task
      // being executed, returning the main thread as soon as possible.
    while(*pFlag == FALSE)
    {
		TASKSETHANDLE handle = mhActiveTaskSets[iReader];

        if(handle != TASKSETHANDLE_INVALID)
        {
            TaskMgrSS::TaskSet *pSet = &gTaskMgrSS.mSets[handle];
            if(pSet->muCompletionCount > 0 && pSet->muTaskId >= 0)
            {
                  // The context ID for the main thread is 0.
                pSet->Execute(0);
            }
            else
            {
                _InterlockedCompareExchange((LONG*)&mhActiveTaskSets[iReader],TASKSETHANDLE_INVALID,handle);
                iReader = (iReader + 1) & (MAX_TASKSETS - 1);
            }
        }
        else if(miTaskCount > 0)
        {
            iReader = (iReader + 1) & (MAX_TASKSETS - 1);
        }
        else
        {
              // Worker threads get suspended, but the main thread needs to stay alert,
              // so it spins until the condition is met or more work it added.
            while(miTaskCount == 0 && *pFlag == FALSE);
        }
    }
}
