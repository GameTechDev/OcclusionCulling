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

#define _CRTDBG_MAP_ALLOC
//#define _CRTDBG_MAP_ALLOC_NEW
#include <stdlib.h>
#include <crtdbg.h>

// Visual Studio leak detection routines
// Only available in debug mode builds.  Reports leak size, allocation #, and
// source file line number
//
// Important note: if you're getting compiler errors in VS2008, you need to
// add the _VS_2008_BUILD_ preprocessor define because there is an incompatiblity
// with the way CRT leak detection defines the new operator and directX headers.
// errors:
// \include\xdebug(32) : warning C4229: anachronism used : modifiers on data are ignored
// \include\xdebug(32) : error C2365: 'operator new' : redefinition; previous definition was 'function'
//-----------------------------------------------------------------------------
#ifndef _VS_2008_BUILD_
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

// define that is the top-level macro for Visual Studio CRT memory debug routines
// it MUST be the very first line of your header file, BEFORE any other includes/etc
//#define _CRT_DEBUG() \ #define _CRTDBG_MAP_ALLOC \ #define _CRTDBG_MAP_ALLOC_NEW \ #include <stdlib.h> \ #include <crtdbg.h> \ #ifdef _DEBUG \ #ifndef DBG_NEW \ #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) \ #define new DBG_NEW \ #endif \ #endif  // _DEBUG
#endif //_VS_2008_BUILD_