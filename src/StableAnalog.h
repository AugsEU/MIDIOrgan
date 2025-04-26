#include <Arduino.h>

#ifndef STABLE_ANALOG_H
#define STABLE_ANALOG_H

struct StableAnalog
{
    uint16_t mStableValue = 0;

    // Update the stabilized value with a new raw reading
    void ConsumeInput(uint16_t analog);
};

#endif // STABLE_ANALOG_H