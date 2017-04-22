/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "CPUTITTTaskMarker.h"


#ifdef CPUT_GPA_INSTRUMENTATION
// Constructor
// automatically creates an ITT begin marker at the start of this task
//-----------------------------------------------------------------------------
CPUTITTTaskMarker::CPUTITTTaskMarker(__itt_domain *pITTDomain, __itt_string_handle *pITTStringHandle)
{
    mpITTDomain = pITTDomain;
    mpITTStringHandle = pITTStringHandle;

    __itt_task_begin(pITTDomain, __itt_null, __itt_null, pITTStringHandle);
}


// Destructor
// When the class goes out of scope, this marker will automatically be called
// and the domain 'closed'
//-----------------------------------------------------------------------------
CPUTITTTaskMarker::~CPUTITTTaskMarker()
{
    __itt_task_end(mpITTDomain);
}
#else
    // This is a bit of a hack to get the compiler not to complain about this being an empty file
    // during the compilation in any mode that doesn't have CPUT_GPA_INSTRUMENTATION defined
    #define CPUTITTTaskMarkerNotEmpty()   namespace { char CPUTITTTaskMarkerDummy##__LINE__; }
    CPUTITTTaskMarkerNotEmpty();
#endif
