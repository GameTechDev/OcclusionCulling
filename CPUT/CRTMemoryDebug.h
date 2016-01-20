//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
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
//--------------------------------------------------------------------------------------

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