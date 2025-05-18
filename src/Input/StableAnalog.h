#include <Arduino.h>

#ifndef STABLE_ANALOG_H
#define STABLE_ANALOG_H

struct StableAnalog
{
    // Update the stabilized value with a new raw reading
    void ConsumeInput(uint16_t analog);
    float ToUnitFloat();
    uint16_t GetStableValue();

private:
    uint16_t mStableValue = 0;
};

#endif // STABLE_ANALOG_H