#include "bits/stdc++.h"

using namespace std;

#pragma once

enum PlatformState
{
    PLATFORM_EMPTY = 0,
    PLATFORM_OCCUPIED = 1,
    PLATFORM_UNDER_SERVICE = 2
};

struct Platform
{
    int platformId;
    PlatformState platformState;

    Platform(int id) : platformId(id)
    {
        platformState = PlatformState::PLATFORM_EMPTY;
    }
};
