/*!
    \file TaskMgrTbb.cpp

    TaskMgrTbb is a class that wraps the TBB library with a C-style handle and 
    callback mechanism for scheduling tasks to scale across any number of CPU
    cores.  This source file contains the implementation of the TaskMgrTBB 
    interface that is used to create task sets and schedule tasks.

    Internal classes used to implement TaskMgrTbb are defined here.  They are
    commented to illustrate their functionality, but are not for use by the 
    applicaton.  All the application needs to use is the TaskMgrTbb interface.

    Copyright 2010 Intel Corporation
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
// #include "SampleComponents.h"
#include "TaskMgrTBB.h"

//  TBB includes
#include <tbb_stddef.h>
#include <task.h>
#include <enumerable_thread_specific.h>
#include <task_scheduler_init.h>
#include <task_scheduler_observer.h>

#include <strsafe.h>

#pragma warning ( push )
#pragma warning ( disable : 4995 ) // skip deprecated warning on intrinsics.
#include <intrin.h>
#pragma warning ( pop )

using namespace tbb;

//
//  Global Domain for GPA CPU tracing
//
#ifdef PROFILEGPA

__itt_domain* g_ProfileDomain = __itt_domain_create( TEXT( "TaskMgr.ProfileDomain" ) );

#endif

//
//  Global TBB task mananger instance
//
TaskMgrTbb                      gTaskMgr;


class SpinLock
{
public:
    SpinLock()
        : muLock( 0 ) 
    {}

    ~SpinLock()
    {}

    VOID
    Lock()
    {
        while( _InterlockedCompareExchange(
            (LONG*)&muLock,
            1,
            0 ) == 1 )
        {}
    }

    VOID
    Unlock()
    {
        muLock = 0;
    }

private:

    volatile UINT               muLock;
};

//
//  Global counter to count whenever a thread registers with
//  TBB and a TLS (Thread-local storage )key to store these indices
//  These indices are the task context ids that will be passed 
//  int to the user-specified task functions.
//
atomic<INT>                gContextIdCount;
enumerable_thread_specific<INT> gContextId;

//
//  INTERNAL
//  The TbbContextId class is an internal implemetation of the
//  TaskMgrTBB class.  The on_scheduler_entry function is called by TBB
//  whenever a new thread is created by TBB.  The function uses an atomic
//  operation to assign each thread a unqiue, zero to n-1 number that can
//  be used by tasks in the tasking system to access thread-local data
//  in an efficient mannor.
//
class TbbContextId : public task_scheduler_observer
{
    void 
    on_scheduler_entry( bool /*bIsWorker*/ )  
    {
        INT iContext = gContextIdCount.fetch_and_increment();
        gContextId.local() = iContext;
    }

public:
    TbbContextId()
    {
        gContextIdCount = 0;
        observe( true );
    }
};

//
//  INTERNAL
//  GenericTask is the wrapper class for individual tbb tasks.  Tasks
//  in TBB are the basic scheduling units.  The class contains all the
//  information needed for the callback.  It also contains a pointer 
//  to the parent tbb task. The parent task (TaskSetTbb defined below)
//  is referenced by GenericTask in order to report completion of the 
//  each GenericTask in the set.
//
class GenericTask : public task
{
public:
    GenericTask()
    : mpFunc( 0 )
    , mpvArg( 0 )
    , muIdx( 0 )
    , muSize( 0 )
    , mpszSetName( NULL )
    , mhTaskSet( TASKSETHANDLE_INVALID )
    {
    };

    GenericTask( 
        TASKSETFUNC         pFunc,
        void*               pvArg,
        UINT                uIdx,
        UINT                uSize,
        CHAR*               pszSetName,
        TASKSETHANDLE       hSet ) 
    : mpFunc( pFunc )
    , mpvArg( pvArg )
    , muIdx( uIdx )
    , muSize( uSize )
    , mpszSetName( pszSetName )
    , mhTaskSet( hSet )
    {
    };

    //  execute will call the app-defined task callback with the 
    //  proper parameters
    task* execute()
    {
        ProfileBeginTask( mpszSetName );

        mpFunc( mpvArg, gContextId.local(), muIdx, muSize );

        ProfileEndTask();

        //  Notify the taskmgr that this set completed one of its tasks.
        gTaskMgr.CompleteTaskSet( mhTaskSet );

        return NULL;
    }

private:

    TASKSETFUNC             mpFunc;
    void*                   mpvArg;
    UINT                    muIdx;
    UINT                    muSize;
    CHAR*                   mpszSetName;

    TASKSETHANDLE           mhTaskSet;
};

//
//  INTERNAL
//  TaskSetTbb is the base tbb task that owns both spawning and tracking
//  the taskset.  It owns the completion count and the successor array.
//  TaskSetTbb will spawn GerericTask instances for each callback the 
//  Application requeseted in TaskMgrTbb::CreateTaskSet.
//
class TaskSetTbb : public task
{
public:
    TaskSetTbb() 
    : mpFunc( NULL )
    , mpvArg( 0 )
    , muSize( 0 )
    , mhTaskset( TASKSETHANDLE_INVALID )
    , mbHasBeenWaitedOn( FALSE )
    {
        mszSetName[ 0 ] = 0;
        memset( Successors, 0, sizeof( Successors ) ) ;
    };

    task* execute()
    {
        //  set the tbb reference count for this TaskSetTbb to
        //  one plus the task set count
        set_ref_count( muSize + 1 );

        ProfileBeginTask("Taskset Spawn Tasks");

        //  Iterate for each task in the set and spawn a GenericTask
        for( UINT uIdx = 0; uIdx < muSize; ++uIdx )
        {
            spawn( *new( allocate_child() ) GenericTask( 
                mpFunc, 
                mpvArg,
                uIdx, 
                muSize,
                mszSetName,
                mhTaskset ) );
        }

        ProfileEndTask();

        return NULL;
    }

    
    TaskSetTbb*             Successors[ MAX_SUCCESSORS ];
    TASKSETHANDLE           mhTaskset;
    BOOL                    mbHasBeenWaitedOn;

    TASKSETFUNC             mpFunc;
    void*                   mpvArg;

    volatile UINT           muStartCount;
    volatile UINT           muCompletionCount;
    volatile UINT           muRefCount;
    
    UINT                    muSize;    
    SpinLock                mSuccessorsLock;

    CHAR                    mszSetName[ MAX_TASKSETNAMELENGTH ];
};

///////////////////////////////////////////////////////////////////////////////
//
//  Implementation of TaskMgrTbb
//
///////////////////////////////////////////////////////////////////////////////

TaskMgrTbb::TaskMgrTbb()
    : mpTbbContextId( NULL )
    , mpTbbInit( NULL )
    , miDemoModeThreadCountOverride( task_scheduler_init::automatic )
{
    memset(
        mSets,
        0x0,
        sizeof( mSets ) );
}

TaskMgrTbb::~TaskMgrTbb()
{
}

BOOL
TaskMgrTbb::Init()
{
    mpTbbContextId = new TbbContextId();

    mpTbbInit = new task_scheduler_init( miDemoModeThreadCountOverride );

    //  Reset thread override demo variable.
    miDemoModeThreadCountOverride = -1;

    return TRUE;
}

VOID
TaskMgrTbb::WaitForAll()
{
    for( UINT uSet = 0; uSet < MAX_TASKSETS; ++uSet )
    {
        if( mSets[ uSet ] )
        {
            WaitForSet( uSet );   

            mSets[ uSet ]->set_ref_count( 0 );
            mSets[ uSet ]->destroy( *mSets[ uSet ] );
            mSets[ uSet ] = NULL;
        }
    }
}

VOID
TaskMgrTbb::Shutdown()
{
    //  
    //  Release any left-over tasksets
    for( UINT uSet = 0; uSet < MAX_TASKSETS; ++uSet )
    {
        if( mSets[ uSet ] )
        {
            WaitForSet( uSet );   

            mSets[ uSet ]->set_ref_count( 0 );
            mSets[ uSet ]->destroy( *mSets[ uSet ] );
            mSets[ uSet ] = NULL;
        }
    }
    
    delete mpTbbContextId;
    delete reinterpret_cast<task_scheduler_init*>(mpTbbInit);
}

BOOL
TaskMgrTbb::CreateTaskSet(
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
    //  Tasksets are spawned when their parents complete.  If no parent for a 
    //  taskset is specified we need to create a fake one.
    //
    //if( 0 == uDepends )
    //{
    //    hSetParent = AllocateTaskSet();
    //    mSets[ hSetParent ]->muCompletionCount = 0;
    //    mSets[ hSetParent ]->muRefCount = 1;

    //    //  Implicit starting task never needs to be waited on for TBB since
    //    //  it is not a real tbb task.
    //    mSets[ hSetParent ]->mbHasBeenWaitedOn = TRUE;

    //    uDepends = 1;
    //    pDepends = &hSetParent;
    //}

    //
    //  Allocate and setup the internal taskset
    //
    hSet = AllocateTaskSet();

    mSets[ hSet ]->muStartCount   = uDepends;

    //  NOTE: one refcount is owned by the tasking system the other 
    //  by the caller.
    mSets[ hSet ]->muRefCount     = 2;

    mSets[ hSet ]->mpFunc         = pFunc;
    mSets[ hSet ]->mpvArg         = pArg;
    mSets[ hSet ]->muSize         = uTaskCount;
    mSets[ hSet ]->muCompletionCount = uTaskCount;
    mSets[ hSet ]->mhTaskset      = hSet;

#ifdef PROFILEGPA
    //
    //  Track task name if profiling is enabled
    if( szSetName )
    {
        StringCbCopyA(
            mSets[ hSet ]->mszSetName,
            sizeof( mSets[ hSet ]->mszSetName ),
            szSetName );
    }
    else
    {
        StringCbCopyA(
            mSets[ hSet ]->mszSetName,
            sizeof( mSets[ hSet ]->mszSetName ),
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
        mSets[ hSet ]->execute();
    }
    else for( UINT uDepend = 0; uDepend < uDepends; ++uDepend )
    {
        TASKSETHANDLE       hDependsOn = pDepends[ uDepend ];

        if(hDependsOn == TASKSETHANDLE_INVALID)
            continue;

        TaskSetTbb*         pDependsOn = mSets[ hDependsOn ];
        LONG                lPrevCompletion;

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

        pDependsOn->mSuccessorsLock.Lock();

        UINT uSuccessor;
        for( uSuccessor = 0; uSuccessor < MAX_SUCCESSORS; ++uSuccessor )
        {
            if( NULL == pDependsOn->Successors[ uSuccessor ] )
            {
                pDependsOn->Successors[ uSuccessor ] = mSets[ hSet ];
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
            pDependsOn->mSuccessorsLock.Unlock();
            goto Cleanup;
        }

        pDependsOn->mSuccessorsLock.Unlock();

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
TaskMgrTbb::ReleaseHandle(
    TASKSETHANDLE           hSet )
{
    _InterlockedDecrement( (LONG*)&mSets[ hSet ]->muRefCount );

    //
    //  Release cannot destroy the object since TBB may still be
    //  referencing internal members.  Defer destruction until
    //  we need to allocate a slot.    
}


VOID
TaskMgrTbb::ReleaseHandles(
    TASKSETHANDLE*              phSet,
    UINT                        uSet )
{
    for( UINT uIdx = 0; uIdx < uSet; ++uIdx )
    {
        ReleaseHandle( phSet[ uIdx ] );
    }
}

VOID
TaskMgrTbb::WaitForSet(
    TASKSETHANDLE               hSet )
{
    //
    //  Yield the main thread to TBB to get our taskset done faster!
    //  NOTE: tasks can only be waited on once.  After that they will
    //  deadlock if waited on again.
    if( !mSets[ hSet ]->mbHasBeenWaitedOn )
    {
        mSets[ hSet ]->wait_for_all();
        mSets[ hSet ]->mbHasBeenWaitedOn = TRUE;
    }

}

TASKSETHANDLE
TaskMgrTbb::AllocateTaskSet()
{
    TaskSetTbb*         pSet = new( task::allocate_root() ) TaskSetTbb();
    UINT                uSet = muNextFreeSet;

    //
    //  Create a new task set and find a slot in the TaskMgrTbb to put it in.
    //
    //  NOTE: if we have too many tasks pending we will spin on the slot.  If
    //  spinning occures, see TaskMgrTbb.h and increase MAX_TASKSETS
    //
    pSet->set_ref_count( 2 );

    //
    //  NOTE: Allocating tasksets is not thread-safe due to allocation of the
    //  slot for the task pointer.  This can be easily made threadsafe with 
    //  an interlocked op on the muNextFreeSet variable and a spin on the slot.  
    //  It will cost a small amount of performance.
    //
    while( NULL != mSets[ uSet ] && 0 != mSets[ uSet ]->muRefCount )
    { 
        uSet = ( uSet + 1 ) % MAX_TASKSETS;
    }
    
    if( NULL != mSets[ uSet ] )
    {
        //  We know the refcount is done, but TBB has an assert that requires
        //  a task be waited on before being deleted.
        WaitForSet( uSet );

        //
        //  Once TaskMgrTbb is done with a tbb object we need to forcibly destroy it.
        //  There are some refcount issues with tasks in tbb 3.0 which can be 
        //  inconsistent if a task has never been waited for.  TaskMgrTbb knows the
        //  correct refcount.
        mSets[ uSet ]->set_ref_count( 0 );
        mSets[ uSet ]->destroy( *mSets[ uSet ] );
        mSets[ uSet ] = NULL;
    }

    mSets[ uSet ] = pSet;
    muNextFreeSet = ( uSet + 1 ) % MAX_TASKSETS;

    return (TASKSETHANDLE)uSet;
}

VOID
TaskMgrTbb::CompleteTaskSet(
    TASKSETHANDLE           hSet )
{
    TaskSetTbb*             pSet = mSets[ hSet ];

    UINT uCount = _InterlockedDecrement( (LONG*)&pSet->muCompletionCount );

    if( 0 == uCount )
    {
        //
        //  The task set has completed.  We need to look at the successors
        //  and signal them that this dependency of theirs has completed.
        //
        pSet->mSuccessorsLock.Lock();

        for( UINT uSuccessor = 0; uSuccessor < MAX_SUCCESSORS; ++uSuccessor )
        {
            TaskSetTbb* pSuccessor = pSet->Successors[ uSuccessor ];

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
                    pSuccessor->execute();
                }
            }
        }

        pSet->mSuccessorsLock.Unlock();

        ReleaseHandle( hSet );
    }
}

BOOL
TaskMgrTbb::IsSetComplete(
    TASKSETHANDLE           hSet )
{
    TaskSetTbb*             pSet = mSets[ hSet ];

    return 0 == pSet->muCompletionCount;
}