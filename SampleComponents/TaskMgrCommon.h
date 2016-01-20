//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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


//  Callback type for tasks in the tasking TaskMgrTBB system
typedef void (*TASKSETFUNC )( void*,
                              int,
                              unsigned int,
                              unsigned int );

//  Handle to a task set that can be used to express task set
//  dependecies and task set synchronization.
typedef unsigned int        TASKSETHANDLE;

//  Value of a TASKSETHANDLE that indicates an invalid handle
#define TASKSETHANDLE_INVALID 0xFFFFFFFF

//
//  Variables to control the memory size and performance of the TaskMgr
//  class.  See header comment for details.
//
#define MAX_SUCCESSORS                  5
#define MAX_TASKSETS                    256
#define MAX_TASKSETNAMELENGTH           512