#pragma once
#include "windows.h"
#pragma warning ( push )
#pragma warning ( disable : 4995 ) // skip deprecated warning on intrinsics.
#include <intrin.h>
#pragma warning ( pop )

#include "Profile.h"

class spin_mutex
{
public:
    volatile long flag;

    spin_mutex() : flag(0) {}

    void aquire()
    {
        while(!try_aquire()) { }
    }

    bool try_aquire()
    {
        return _InterlockedCompareExchange(&flag,1,0) == 0;
    }

    void release()
    {
        _InterlockedExchange(&flag,0);
    }
};

