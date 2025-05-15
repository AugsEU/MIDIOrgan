#include <AugSynth.h>
#include <AugSynthParams.h>

/// ===================================================================================
/// Globals
/// ===================================================================================

AugSynthParam gAugSynthParams[ASP_NUM_PARAMS];

AugSynthDial gDial1;
AugSynthDial gDial2;
AugSynthDial gDial3;  // 1 2
AugSynthDial gDial4;  // 3 4
AugSynthDial gDial5;  // 5 6
AugSynthDial gDial6;

RotaryEncoder gPageSelect;
uint8_t gCurrParamPage;

/// ===================================================================================
/// AugSynthParam
/// ===================================================================================
AugSynthParam::AugSynthParam(uint8_t paramNum, int8_t value, int8_t minValue, int8_t maxValue)
{
    mParamNum = paramNum;
    mValue = value;
    mMaxValue = maxValue;
    mMinValue = minValue;
}

void AugSynthParam::ApplyDelta(int8_t delta)
{
    mValue += delta;
    if(mValue > mMaxValue)
    {
        mValue = mMaxValue;
    }
    else if(mValue < mMinValue)
    {
        mValue = mMinValue;
    }
}

float AugSynthParam::GetFloatValue()
{
    return 0.0f;
}

/// ===================================================================================
/// AugSynthDial
/// ===================================================================================
AugSynthDial::AugSynthDial()
{
    for(uint8_t i = 0; i < NUM_SYNTH_PAGES; i++)
    {
        mParamters[i] = nullptr;
        mShiftParamters[i] = nullptr;
    }
}

void AugSynthDial::UpdateValue(bool left, bool right, bool pressed)
{
    AugSynthParam* pParam = pressed ? mShiftParamters[gCurrParamPage] : mParamters[gCurrParamPage];
    int8_t delta = mEncoder.UpdateDial(left, right);

    if(pParam != nullptr)
    {
        pParam->ApplyDelta(delta);
    }
}

/// ===================================================================================
/// Public functions
/// ===================================================================================
void InitAugSynth()
{

}

void InitSynthPatch()
{
    // General          Type                                                     Default  min  max
    gAugSynthParams[ASP_TUNING             ] = AugSynthParam(ASP_TUNING             , 0,  0,   13);
    gAugSynthParams[ASP_DRIVE              ] = AugSynthParam(ASP_DRIVE              , 0,  0,   50);
    gAugSynthParams[ASP_GAIN               ] = AugSynthParam(ASP_GAIN               , 25, 0,   50);
    gAugSynthParams[ASP_DELAY_TIME         ] = AugSynthParam(ASP_DELAY_TIME         , 0,  0,   50);
    gAugSynthParams[ASP_DELAY_FEEDBACK     ] = AugSynthParam(ASP_DELAY_FEEDBACK     , 0,  0,   50);
    gAugSynthParams[ASP_DELAY_SHEAR        ] = AugSynthParam(ASP_DELAY_SHEAR        , 0,  0,   50);
    gAugSynthParams[ASP_DELAY_MODE         ] = AugSynthParam(ASP_DELAY_MODE         , 0,  0,   3); // Normal, Slapback, Octave, Reverse
 
    // DCO 
    gAugSynthParams[ASP_DCO_WAVE_SHAPE_1   ] = AugSynthParam(ASP_DCO_WAVE_SHAPE_1   , 0,  0,   5); // Sine, Square, Saw, Triangle, Ramp, Glitch, Noise
    gAugSynthParams[ASP_DCO_TUNE_1         ] = AugSynthParam(ASP_DCO_TUNE_1         , 0,  -50, 50);
    gAugSynthParams[ASP_DCO_VOL_1          ] = AugSynthParam(ASP_DCO_VOL_1          , 50,  0,  50);
    gAugSynthParams[ASP_DCO_PW_1           ] = AugSynthParam(ASP_DCO_PW_1           , 10, 0,   20);
    gAugSynthParams[ASP_DCO_PWM_1          ] = AugSynthParam(ASP_DCO_PWM_1          , 0,  -20, 20);
    gAugSynthParams[ASP_DCO_WAVE_SHAPE_2   ] = AugSynthParam(ASP_DCO_WAVE_SHAPE_2   , 0,  0,   5); // Sine, Square, Saw, Triangle, Ramp, Glitch, Noise
    gAugSynthParams[ASP_DCO_TUNE_2         ] = AugSynthParam(ASP_DCO_TUNE_2         , 0,  -50, 50);
    gAugSynthParams[ASP_DCO_VOL_2          ] = AugSynthParam(ASP_DCO_VOL_2          , 0,  0,   50);
    gAugSynthParams[ASP_DCO_PW_1           ] = AugSynthParam(ASP_DCO_PW_1           , 10, 0,   20);
    gAugSynthParams[ASP_DCO_PWM_1          ] = AugSynthParam(ASP_DCO_PWM_1          , 0,  -20, 20);

    // ENV
    gAugSynthParams[ASP_ENV_ATTACK1        ] = AugSynthParam(ASP_ENV_ATTACK1        , 0,  45,  50); //@TODO Make 0 no delay
    gAugSynthParams[ASP_ENV_DECAY1         ] = AugSynthParam(ASP_ENV_DECAY1         , 0,  40,  50);
    gAugSynthParams[ASP_ENV_SUSTAIN1       ] = AugSynthParam(ASP_ENV_SUSTAIN1       , 0,  40,  50);
    gAugSynthParams[ASP_ENV_RELEASE1       ] = AugSynthParam(ASP_ENV_RELEASE1       , -1, -1,  50); // Note: -1 is decay
    gAugSynthParams[ASP_ENV_ATTACK2        ] = AugSynthParam(ASP_ENV_ATTACK2        , 0,  45,  50);
    gAugSynthParams[ASP_ENV_SUSTAIN2       ] = AugSynthParam(ASP_ENV_SUSTAIN2       , 0,  40,  50);
    gAugSynthParams[ASP_ENV_DECAY2         ] = AugSynthParam(ASP_ENV_DECAY2         , 0,  40,  50);
    gAugSynthParams[ASP_ENV_RELEASE2       ] = AugSynthParam(ASP_ENV_RELEASE2       , -1, -1,  50); // Note -1 is decay
   
    // VCF   
    gAugSynthParams[ASP_VCF_CUTOFF         ] = AugSynthParam(ASP_VCF_CUTOFF         , 0,  0,   50);
    gAugSynthParams[ASP_VCF_RES            ] = AugSynthParam(ASP_VCF_RES            , 0,  0,   20);
    gAugSynthParams[ASP_VCF_MODE           ] = AugSynthParam(ASP_VCF_MODE           , 0,  0,   3);  // Off, LP, BP, HP
    gAugSynthParams[ASP_VCF_CUTOFF_LFO     ] = AugSynthParam(ASP_VCF_CUTOFF_LFO     , 0,  -20, 20);
    gAugSynthParams[ASP_VCF_RES_LFO        ] = AugSynthParam(ASP_VCF_RES_LFO        , 0,  -20, 20);
    gAugSynthParams[ASP_VCF_FOLLOW         ] = AugSynthParam(ASP_VCF_FOLLOW         , 0,  0,   50);
   
    // LFO   
    gAugSynthParams[ASP_LFO_RATE           ] = AugSynthParam(ASP_LFO_RATE           , 0,  0,   99);
    gAugSynthParams[ASP_LFO_WAVE_SHAPE     ] = AugSynthParam(ASP_LFO_WAVE_SHAPE     , 0,  0,   99);
    gAugSynthParams[ASP_LFO_ATTACK         ] = AugSynthParam(ASP_LFO_ATTACK         , 0,  0,   99);
    gAugSynthParams[ASP_LFO_GAIN           ] = AugSynthParam(ASP_LFO_GAIN           , 0,  0,   99);
    gAugSynthParams[ASP_LFO_OSC1_TUNE      ] = AugSynthParam(ASP_LFO_OSC1_TUNE      , 0,  0,   99);
    gAugSynthParams[ASP_LFO_OSC1_VOLUME    ] = AugSynthParam(ASP_LFO_OSC1_VOLUME    , 0,  0,   99);
    gAugSynthParams[ASP_LFO_OSC2_TUNE      ] = AugSynthParam(ASP_LFO_OSC2_TUNE      , 0,  0,   99);
    gAugSynthParams[ASP_LFO_OSC1_VOLUME    ] = AugSynthParam(ASP_LFO_OSC1_VOLUME    , 0,  0,   99);
} 

void UpdateAugSynth()
{

}