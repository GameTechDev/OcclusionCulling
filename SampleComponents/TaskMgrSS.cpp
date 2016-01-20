/*!
    \file TaskMgrSS.h

    TaksMgrSS is a class that uses a custom Windows threads schedulers with a
    C-style handle and callback mechanism for scheduling tasks across any 
    number of CPU cores.

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
#include "TaskMgrSS.h"

#include "TaskScheduler.h"
#include "spin_mutex.h"

#include <strsafe.h>

#pragma warning ( push )
#pragma warning ( disable : 4995 ) // skip deprecated warning on intrinsics.
#include <intrin.h>
#pragma warning ( pop )

//
//  Global Domain for GPA CPU tracing
//
#ifdef PROFILEGPA

__itt_domain* g_ProfileDomain = __itt_domain_create( TEXT( "TaskMgr.ProfileDomain" ) );

#endif

//
//  Global Simple Scheduler task mananger instance
//
TaskMgrSS                      gTaskMgrSS;


TaskMgrSS::TaskSet::TaskSet() 
: mpFunc( NULL )
, mpvArg( 0 )
, muSize( 0 )
, mhTaskset( TASKSETHANDLE_INVALID )
, mbCompleted( TRUE )
{
    mszSetName[ 0 ] = 0;
    memset( Successors, 0, sizeof( Successors ) ) ;
};

void TaskMgrSS::TaskSet::Execute(INT iContextId)
{
    int uIdx = _InterlockedDecrement(&muTaskId);
    if(uIdx >= 0)
    {
        //gTaskMgrSS.mTaskScheduler.DecrementTaskCount();

        ProfileBeginTask( mszSetName );

        mpFunc( mpvArg, iContextId, uIdx, muSize );

        ProfileEndTask();

        //gTaskMgr.CompleteTaskSet( mhTaskset );
        UINT uCount = _InterlockedDecrement( (LONG*)&muCompletionCount );

        if( 0 == uCount )
        {
            mbCompleted = TRUE;
            mpFunc = 0;
            CompleteTaskSet();
        }
    }
}

void TaskMgrSS::TaskSet::CompleteTaskSet()
{
    //
    //  The task set has completed.  We need to look at the successors
    //  and signal them that this dependency of theirs has completed.
    //

    mSuccessorsLock.aquire();

    for( UINT uSuccessor = 0; uSuccessor < MAX_SUCCESSORS; ++uSuccessor )
    {
        TaskSet* pSuccessor = Successors[ uSuccessor ];

        //
        //  A signaled successor must be removed from the Successors list 
        //  before the mSuccessorsLock can be released.
        //
        Successors[ uSuccessor ] = NULL;
            
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
                gTaskMgrSS.mTaskScheduler.AddTaskSet( pSuccessor->mhTaskset, pSuccessor->muSize );
            }
        }
    }

    mSuccessorsLock.release();

    gTaskMgrSS.ReleaseHandle( mhTaskset );
}

///////////////////////////////////////////////////////////////////////////////
//
//  Implementation of TaskMgrSS
//
///////////////////////////////////////////////////////////////////////////////

TaskMgrSS::TaskMgrSS() : miDemoModeThreadCountOverride(-1)
{
    memset(
        mSets,
        0x0,
        sizeof( mSets ) );
}

TaskMgrSS::~TaskMgrSS()
{
}

BOOL TaskMgrSS::Init()
{
    mTaskScheduler.Init(miDemoModeThreadCountOverride);

    return TRUE;
}

VOID TaskMgrSS::Shutdown()
{
    //  
    //  Release any left-over tasksets
    for( UINT uSet = 0; uSet < MAX_TASKSETS; ++uSet )
    {
        if( mSets[ uSet ].mpFunc )
        {
            WaitForSet( uSet );   
        }
    }

    mTaskScheduler.Shutdown();
}


BOOL TaskMgrSS::CreateTaskSet(TASKSETFUNC     pFunc,
                              VOID*           pArg,
                              UINT            uTaskCount,
                              TASKSETHANDLE*  pInDepends,
                              UINT            uInDepends,
                              OPTIONAL LPCSTR szSetName,
                              TASKSETHANDLE*  pOutHandle )
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

    mSets[ hSet ].muRefCount        = 2;
    mSets[ hSet ].muStartCount      = uDepends;
    mSets[ hSet ].mpvArg            = pArg;
    mSets[ hSet ].muSize            = uTaskCount;
    mSets[ hSet ].muCompletionCount = uTaskCount;
    mSets[ hSet ].muTaskId          = uTaskCount;
    mSets[ hSet ].mhTaskset         = hSet;
    mSets[ hSet ].mpFunc            = pFunc;
    mSets[ hSet ].mbCompleted       = FALSE;
    //mSets[ hSet ].mhAssignedSlot    = TASKSETHANDLE_INVALID;

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
	if(uDepends == 0)
	{
		mTaskScheduler.AddTaskSet( hSet, uTaskCount );
	}
    else for( UINT uDepend = 0; uDepend < uDepends; ++uDepend )
    {
        TASKSETHANDLE       hDependsOn = pDepends[ uDepend ];

        if(hDependsOn == TASKSETHANDLE_INVALID)
            continue;

        TaskSet *pDependsOn = &mSets[ hDependsOn ];
        LONG     lPrevCompletion;

        //
        //  A taskset with a new successor is consider incomplete even if it
        //  already has completed.  This mechanism allows us tasksets that are
        //  already done to appear active and capable of spawning successors.
        //
        lPrevCompletion = _InterlockedExchangeAdd( (LONG*)&pDependsOn->muCompletionCount, 1 );

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

VOID TaskMgrSS::ReleaseHandle( TASKSETHANDLE hSet )
{
    _InterlockedDecrement( (LONG*)&mSets[ hSet ].muRefCount );
}


VOID TaskMgrSS::ReleaseHandles( TASKSETHANDLE *phSet,UINT uSet )
{
    for( UINT uIdx = 0; uIdx < uSet; ++uIdx )
    {
        ReleaseHandle( phSet[ uIdx ] );
    }
}

VOID TaskMgrSS::WaitForSet( TASKSETHANDLE               hSet )
{
    //
    //  Yield the main thread to SS to get our taskset done faster!
    //  NOTE: tasks can only be waited on once.  After that they will
    //  deadlock if waited on again.
    if( !mSets[ hSet ].mbCompleted )
    {
        mTaskScheduler.WaitForFlag(&mSets[ hSet ].mbCompleted);
    }

}

BOOL
TaskMgrSS::IsSetComplete( TASKSETHANDLE hSet )
{
    return TRUE == mSets[ hSet ].mbCompleted;
}


TASKSETHANDLE TaskMgrSS::AllocateTaskSet()
{
      // This line is fine
    UINT                        uSet = muNextFreeSet;

    //
    //  Create a new task set and find a slot in the TaskMgrSS to put it in.
    //
    //  NOTE: if we have too many tasks pending we will spin on the slot.  If
    //  spinning occures, see TaskMgrCommon.h and increase MAX_TASKSETS
    //

    //
    //  NOTE: Allocating tasksets is not thread-safe due to allocation of the
    //  slot for the task pointer.  This can be easily made threadsafe with 
    //  an interlocked op on the muNextFreeSet variable and a spin on the slot.  
    //  It will cost a small amount of performance.
    //
    while( NULL != mSets[ uSet ].mpFunc && mSets[ uSet ].muRefCount != 0 )
    { 
        uSet = ( uSet + 1 ) % MAX_TASKSETS;
    }

    muNextFreeSet = ( uSet + 1 ) & (MAX_TASKSETS - 1);

    return (TASKSETHANDLE)uSet;
}

VOID TaskMgrSS::CompleteTaskSet( TASKSETHANDLE hSet )
{
    TaskSet*             pSet = &mSets[ hSet ];

    UINT uCount = _InterlockedDecrement( (LONG*)&pSet->muCompletionCount );

    if( 0 == uCount )
    {
        pSet->mbCompleted = TRUE;
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
                    mTaskScheduler.AddTaskSet( pSuccessor->mhTaskset, pSuccessor->muSize );
                }
            }
        }

        pSet->mSuccessorsLock.release();

        ReleaseHandle( hSet );
    }
}
