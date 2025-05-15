#include <Arduino.h>
#include <Input/RotaryEncoder.h>
#include <math.h>

#ifndef AUG_SYNTH_H
#define AUG_SYNTH_H

constexpr uint8_t NUM_SYNTH_PAGES = 5;

struct AugSynthParam
{
    uint8_t mParamNum;
    int8_t mValue;
    int8_t mMaxValue;
    int8_t mMinValue;

    AugSynthParam(uint8_t paramNum, int8_t value, int8_t minValue, int8_t maxValue);
    void ApplyDelta(int8_t delta);

    float GetFloatValue();
};

struct AugSynthDial
{
    RotaryEncoder mEncoder;
    AugSynthParam* mParamters[NUM_SYNTH_PAGES];
    AugSynthParam* mShiftParamters[NUM_SYNTH_PAGES];

    AugSynthDial();

    void UpdateValue(bool left, bool right, bool pressed);
};

void InitAugSynth();
void InitSynthPatch();
void UpdateAugSynth();

#endif // AUG_SYNTH_H