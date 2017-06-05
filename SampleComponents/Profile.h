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

/*! Intel Graphics Performance Analyizer (GPA) allows for CPU tracing of tasks
    in a frame.  Define PROFILEGPA to send task notifications to GPA.  
    BeginTask/EndTask are nops if profiling is disabled.
*/
#ifdef PROFILEGPA

#include <ittnotify.h>

extern __itt_domain* g_ProfileDomain;

#define ProfileBeginTask( name )    __itt_task_begin( g_ProfileDomain, __itt_null, __itt_null, __itt_string_handle_createA( name ) )
#define ProfileEndTask()            __itt_task_end( g_ProfileDomain )
#define ProfileBeginFrame( name )   __itt_task_begin( g_ProfileDomain, __itt_null, __itt_null, __itt_string_handle_createA( name ) )
#define ProfileEndFrame()           __itt_task_end( g_ProfileDomain )
#define ProfileAddMarker( text, scope )    __itt_marker(g_ProfileDomain, __itt_null, __itt_string_handle_createA( text ), ITT_JOIN(__itt_marker_scope_,scope) )

#else

#define ProfileBeginTask( name )
#define ProfileEndTask()
#define ProfileBeginFrame( name )
#define ProfileEndFrame()
#define ProfileAddMarker( text, scope )
#endif
