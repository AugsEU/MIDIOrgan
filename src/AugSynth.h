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
    uint8_t mParamNum;

    AugSynthParam() {};
    AugSynthParam(uint8_t paramNum, int8_t maxValue);
    void ApplyDelta(int8_t delta);

    float GetNormFloatValue(int8_t value);
    static float ScaleFloat(uint8_t paramNum, float input);

    int8_t GetMaxValue();
    int8_t GetMinValue();
    int8_t& GetValue();

    void SendValueToBpSynth();

private:
    int8_t mBound;
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
void LoadPresetToAugSynth();

AugSynthPageParams& GetCurrPageParams();
#endif // AUG_SYNTH_H