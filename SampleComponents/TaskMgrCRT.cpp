/*!
    \file TaskMgrCRT.cpp
    \file TaskMgrTbb.cpp

    TaskMgrTbb is a class that wraps the CRT library with a C-style handle and 
    TaskMgrTbb is a class that wraps the TBB library with a C-style handle and 
    callback mechanism for scheduling tasks to scale across any number of CPU
    cores.  This source file contains the implementation of the TaskMgrTBB 
    interface that is used to create task sets and schedule tasks.

    Internal classes used to implement TaskMgrCRT are defined here.  They are
    commented to illustrate their functionality, but are not for use by the 
    applicaton.  All the application needs to use is the TaskMgrTbb interface.

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
#include "SampleComponents.h"
#include "TaskMgrCRT.h"

#include <concrt.h>
#include <concrtrm.h>
#include <ppl.h>

#include <strsafe.h>

#pragma warning ( push )
#pragma warning ( disable : 4995 ) // skip deprecated warning on intrinsics.
#include <intrin.h>
#pragma warning ( pop )

#include <enumerable_thread_specific.h>

namespace
{
    tbb::atomic<INT>                gContextIdCount;
    tbb::enumerable_thread_specific<INT> gContextId;
}

#ifdef PROFILEGPA

//__itt_domain* domain = __itt_domain_create( TEXT( "Traces.MultiThreadedAnimation" ) );

#endif

//
//  Global CRT task mananger instance
//
TaskMgrCRT                      gTaskMgrCRT;

//
//  INTERNAL
//  TaskSetCRT is the base tbb task that owns both spawning and tracking
//  the taskset.  It owns the completion count and the successor array.
//

TaskMgrCRT::TaskSet::TaskSet() 
: mpFunc( NULL )
, mpvArg( 0 )
, muSize( 0 )
, mhTaskset( TASKSETHANDLE_INVALID )
, mbHasBeenWaitedOn( FALSE )
{
    mszSetName[ 0 ] = 0;
    memset( Successors, 0, sizeof( Successors ) ) ;
};

void TaskMgrCRT::TaskSet::SpawnTasks()
{
    for( UINT uIdx = 0; uIdx < muSize; ++uIdx )
    {
        mTaskGroup.run([this,uIdx]() { this->ExecuteTask(uIdx); });
    }
}

void TaskMgrCRT::TaskSet::ExecuteTask(int TaskId)
{
    bool exsists = false;
    INT iContextId = gContextId.local(exsists);
    if(exsists == false)
    {
        iContextId = gContextIdCount.fetch_and_increment();
        gContextId.local() = iContextId;
    }

    ProfileBeginTask( mszSetName );

    //UINT uIdx = _InterlockedIncrement((LONG*)&muTaskId) - 1;

    mpFunc( mpvArg, gContextId.local(), TaskId, muSize );

    ProfileEndTask();

    //  Notify the taskmgr that this set completed one of its tasks.
    gTaskMgrCRT.CompleteTaskSet( mhTaskset );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Implementation of TaskMgrCRT
//
///////////////////////////////////////////////////////////////////////////////

TaskMgrCRT::TaskMgrCRT() : miDemoModeThreadCountOverride(-1)
{
    memset(
        mSets,
        0x0,
        sizeof( mSets ) );
}

TaskMgrCRT::~TaskMgrCRT()
{
}

BOOL
TaskMgrCRT::Init()
{
    gContextIdCount = 0;

    Concurrency::SchedulerPolicy policy(0);
    if(miDemoModeThreadCountOverride != -1)
        policy.SetPolicyValue(Concurrency::MaxConcurrency,miDemoModeThreadCountOverride);
    Concurrency::CurrentScheduler::Create(policy);

    //  Reset thread override demo variable.
    miDemoModeThreadCountOverride = -1;

    return TRUE;
}

VOID
TaskMgrCRT::Shutdown()
{
    //  
    //  Release any left-over tasksets
    for( UINT uSet = 0; uSet < MAX_TASKSETS; ++uSet )
    {
        if( mSets[ uSet ].mpFunc != 0 )
        {
            WaitForSet( uSet );   
        }
    }

    Concurrency::CurrentScheduler::Detach();
}

#include <new>

BOOL
TaskMgrCRT::CreateTaskSet(
    TASKSETFUNC             pFunc,
    VOID*                   pArg,
    UINT                    uTaskCount,
    TASKSETHANDLE*          pInDepends,
    UINT                    uInDepends,
    OPTIONAL LPCSTR         szSetName,
    TASKSETHANDLE*          pOutHandle )
{
    TASKSETHANDLE           hSet;
    TASKSETHANDLE           hSetParent = TASKSETHANDLE_INVALID;
    TASKSETHANDLE*          pDepends = pInDepends;
    UINT                    uDepends = uInDepends;
    BOOL                    bResult = FALSE;

    //  Validate incomming parameters
    if( 0 == uTaskCount || NULL == pFunc )
    {
        return FALSE;
    }

    //
    //  Allocate and setup the internal taskset
    //
    hSet = AllocateTaskSet();

      // Construct a new task set in the slot
    new(&mSets[ hSet ]) TaskSet();

    mSets[ hSet ].muStartCount   = uDepends;

    //  NOTE: one refcount is owned by the tasking system the other 
    //  by the caller.
    mSets[ hSet ].muRefCount     = 2;

    mSets[ hSet ].mpFunc            = pFunc;
    mSets[ hSet ].mpvArg            = pArg;
    mSets[ hSet ].muSize            = uTaskCount;
    mSets[ hSet ].muCompletionCount = uTaskCount;
    mSets[ hSet ].mhTaskset         = hSet;

#ifdef PROFILEGPA
    //
    //  Track task name if profiling is enabled
    if( szSetName )
    {
        StringCbCopyA(
            mSets[ hSet ].mszSetName,
            sizeof( mSets[ hSet ].mszSetName ),
            szSetName );
    }
    else
    {
        StringCbCopyA(
            mSets[ hSet ].mszSetName,
            sizeof( mSets[ hSet ].mszSetName ),
            "Unnamed Task" );
    }
#else
    UNREFERENCED_PARAMETER( szSetName );
#endif // PROFILEGPA

    //
    //  Iterate over the dependency list and setup the successor
    //  pointers in each parent to point to this taskset.
    //
    if( 0 == uDepends )
    {
        mSets[hSet].SpawnTasks();
    }
    else for( UINT uDepend = 0; uDepend < uDepends; ++uDepend )
    {
        TASKSETHANDLE hDependsOn = pDepends[ uDepend ];
        TaskSet*      pDependsOn = &mSets[ hDependsOn ];
        LONG          lPrevCompletion;

        //
        //  A taskset with a new successor is consider incomplete even if it
        //  already has completed.  This mechanism allows us tasksets that are
        //  already done to appear active and capable of spawning successors.
        //
        lPrevCompletion = _InterlockedExchangeAdd( (LONG*)&pDependsOn->muCompletionCount, 1 );

        if( 0 == lPrevCompletion && hSetParent != hDependsOn )
        {
            //  The dependency taskset was already completed.  This means we have,
            //  or will soon, release the refcount for the tasking system.  Addref
            //  the taskset since the next Completion will release it.
            //  This does not apply to the system-created parent.  
            //
            //  NOTE: There is no race conditon here since the caller must still
            //  hold a reference to the depenent taskset which was passed in.
            _InterlockedIncrement( (LONG*)&pDependsOn->muRefCount );
        }

        pDependsOn->mSuccessorsLock.aquire();

        UINT uSuccessor;
        for( uSuccessor = 0; uSuccessor < MAX_SUCCESSORS; ++uSuccessor )
        {
            if( NULL == pDependsOn->Successors[ uSuccessor ] )
            {
                pDependsOn->Successors[ uSuccessor ] = &mSets[ hSet ];
                break;
            }
        }

        //
        //  If the successor list is full we have a problem.  The app
        //  needs to give us more space by increasing MAX_SUCCESSORS
        //
        if( uSuccessor == MAX_SUCCESSORS )
        {
            printf( "Too many successors for this task set.\nIncrease MAX_SUCCESSORS\n" );
            pDependsOn->mSuccessorsLock.release();
            goto Cleanup;
        }

        pDependsOn->mSuccessorsLock.release();

        //  
        //  Mark the set as completed for the successor adding operation.
        //
        CompleteTaskSet( hDependsOn );
    }

    //  Set output taskset handle
    *pOutHandle = hSet;

    bResult = TRUE;

Cleanup:

    return bResult;
}

VOID
TaskMgrCRT::ReleaseHandle(
    TASKSETHANDLE           hSet )
{
    _InterlockedDecrement( (LONG*)&mSets[ hSet ].muRefCount );

    //
    //  Release cannot destroy the object since TBB may still be
    //  referencing internal members.  Defer destruction until
    //  we need to allocate a slot.    
}


VOID
TaskMgrCRT::ReleaseHandles(
    TASKSETHANDLE*              phSet,
    UINT                        uSet )
{
    for( UINT uIdx = 0; uIdx < uSet; ++uIdx )
    {
        ReleaseHandle( phSet[ uIdx ] );
    }
}

VOID
TaskMgrCRT::WaitForSet(
    TASKSETHANDLE               hSet )
{
    //
    //  Yield the main thread to CRT to get our taskset done faster!
    //  NOTE: tasks can only be waited on once.  After that they will
    //  deadlock if waited on again.
    if( !mSets[ hSet ].mbHasBeenWaitedOn )
    {
        mSets[ hSet ].mTaskGroup.wait();
        mSets[ hSet ].mbHasBeenWaitedOn = TRUE;
    }

}

BOOL
TaskMgrCRT::IsSetComplete(
    TASKSETHANDLE           hSet )
{
    return NULL == mSets[ hSet ].mpFunc || 0 == mSets[ hSet ].muRefCount;
}

TASKSETHANDLE
TaskMgrCRT::AllocateTaskSet()
{
    UINT                uSet = muNextFreeSet;

    //
    //  NOTE: Allocating tasksets is not thread-safe due to allocation of the
    //  slot for the task pointer.  This can be easily made threadsafe with 
    //  an interlocked op on the muNextFreeSet variable and a spin on the slot.  
    //  It will cost a small amount of performance.
    //
    while( NULL != mSets[ uSet ].mpFunc && 0 != mSets[ uSet ].muRefCount )
    { 
        uSet = ( uSet + 1 ) % MAX_TASKSETS;
    }
    
    if( NULL != mSets[ uSet ].mpFunc )
    {
        WaitForSet( uSet );
    }

    muNextFreeSet = ( uSet + 1 ) % MAX_TASKSETS;

    return (TASKSETHANDLE)uSet;
}

VOID
TaskMgrCRT::CompleteTaskSet(
    TASKSETHANDLE           hSet )
{
    TaskSet *pSet = &mSets[ hSet ];

    UINT uCount = _InterlockedDecrement( (LONG*)&pSet->muCompletionCount );

    if( 0 == uCount )
    {
        pSet->mpFunc = 0;
        //
        //  The task set has completed.  We need to look at the successors
        //  and signal them that this dependency of theirs has completed.
        //
        pSet->mSuccessorsLock.aquire();

        for( UINT uSuccessor = 0; uSuccessor < MAX_SUCCESSORS; ++uSuccessor )
        {
            TaskSet* pSuccessor = pSet->Successors[ uSuccessor ];

            //
            //  A signaled successor must be removed from the Successors list 
            //  before the mSuccessorsLock can be released.
            //
            pSet->Successors[ uSuccessor ] = NULL;
            
            if( NULL != pSuccessor ) 
            {
                UINT uStart;

                uStart = _InterlockedDecrement( (LONG*)&pSuccessor->muStartCount );

                //
                //  If the start count is 0 the successor has had all its 
                //  dependencies satisified and can be scheduled.
                //
                if( 0 == uStart )
                {
                    pSuccessor->SpawnTasks();
                }
            }
        }

        pSet->mSuccessorsLock.release();
        
        ReleaseHandle( hSet );
    }
}
