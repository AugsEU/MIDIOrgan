#include <Input/StableAnalog.h>
#include <UserControls.h>

constexpr uint32_t ALPHA = 16; // Smoothing factor

void StableAnalog::ConsumeInput(uint16_t analog) 
{
    analog <<= 4;

    uint32_t analog32 = (uint32_t)analog;
    // Lerp between points
    uint32_t weightedInput = analog32 * ALPHA;
    uint32_t weightedStable = (uint32_t)mStableValue * (256 - ALPHA);

    mStableValue = (weightedInput + weightedStable + 128) >> 8;
}

uint16_t StableAnalog::GetStableValue()
{
    return mStableValue >> 4;
}

float StableAnalog::ToUnitFloat()
{
    return GetStableValue() / (float)ANALOG_MAX_VALUE;
}