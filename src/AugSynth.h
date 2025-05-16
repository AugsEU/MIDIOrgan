#include <Arduino.h>
#include <math.h>
#include <AugSynthParams.h>

#ifndef AUG_SYNTH_H
#define AUG_SYNTH_H

struct AugSynthParam
{
    uint8_t mParamNum;
    int8_t mValue;
    int8_t mMaxValue;
    int8_t mMinValue;

    AugSynthParam() {};
    AugSynthParam(uint8_t paramNum, int8_t value, int8_t minValue, int8_t maxValue);
    void ApplyDelta(int8_t delta);

    void WriteToScreenBuff(char* buff);

    float GetFloatValue();

    void SendValueToBpSynth();
};

struct AugSynthDial
{
    AugSynthParam* mParamters[AugSynthPage::NUM_SYNTH_PAGES];
    AugSynthParam* mShiftParamters[AugSynthPage::NUM_SYNTH_PAGES];

    AugSynthDial();

    void UpdateValue(int8_t delta, bool pressed, char* buff, bool forceSend);
};

void InitAugSynth();
void InitSynthPatch();
void BindDialsToParams();
void SendAllParams();

void UpdateAugSynth();

#endif // AUG_SYNTH_H