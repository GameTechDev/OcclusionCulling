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
