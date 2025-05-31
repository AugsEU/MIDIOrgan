#include <Arduino.h>
#include <math.h>
#include <AugSynthParams.h>

#ifndef AUG_SYNTH_H
#define AUG_SYNTH_H

struct AugSynthParam;

constexpr uint8_t NUM_SYNTH_DIALS = 4;
extern AugSynthParam gAugSynthParams[ASP_NUM_PARAMS];

struct AugSynthParam
{
    uint8_t mParamNum; //@TODO get rid of this?
    int8_t mValue;
    int8_t mMaxValue;
    int8_t mMinValue;

    AugSynthParam() {};
    AugSynthParam(uint8_t paramNum, int8_t value, int8_t minValue, int8_t maxValue);
    void ApplyDelta(int8_t delta);

    float GetNormFloatValue();
    static float ScaleFloat(uint8_t paramNum, float input);

    void SendValueToBpSynth();
};

struct AugSynthPageParams
{
    AugSynthPageType mPageType;
    AugSynthParam* mParameters[NUM_SYNTH_DIALS];

    AugSynthPageParams();

    void UpdateValues();
};

void InitAugSynth();
void InitSynthPatch();
void BindSynthPages();
void SendAllParams();
void UpdateAugSynth();

AugSynthPageParams& GetCurrPageParams();
#endif // AUG_SYNTH_H