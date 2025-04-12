#include <Arduino.h>

#ifndef TIME_INFO_H
#define TIME_INFO_H

using uTimeMs = unsigned long; // Unsigned time in milliseconds type.

struct TimeInfo
{
    uTimeMs mTimeMs = 0;

    void PollTime();
};

#endif // TIME_INFO_H