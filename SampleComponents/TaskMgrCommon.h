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