#include <Input/StableAnalog.h>
#include <UserControls.h>

constexpr uint32_t ALPHA = 32; // Smoothing factor

void StableAnalog::ConsumeInput(uint16_t analog) 
{
    // Lerp between points 921  891
    uint32_t weightedInput = (uint32_t)analog * ALPHA;
    uint32_t weightedStable = (uint32_t)mStableValue * (256 - ALPHA);

    mStableValue = (weightedInput + weightedStable + 128) >> 8;
}

float StableAnalog::ToUnitFloat()
{
    return mStableValue / (float)ANALOG_MAX_VALUE;
}