#include <Arduino.h>
#include <TimeInfo.h>

void TimeInfo::PollTime()
{
    mTimeMs = millis();
}