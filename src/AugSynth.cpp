#include <AugSynth.h>
#include <AugSynthParams.h>
#include <LedControl.h>
#include <UserControls.h>

/// ===================================================================================
/// Globals
/// ===================================================================================

constexpr uint8_t NUM_SYNTH_DIALS = 6;
constexpr uint8_t NUM_LED_PANELS = 3;

constexpr uint8_t VPIN_PAGE_SELECT_SW = 6;
constexpr uint8_t PAGE_SELECT_DIAL_IDX = 0;

constexpr uint8_t VPIN_DIALS_SW[NUM_SYNTH_DIALS] = {1, 0, 3, 5, 2, 4};
constexpr uint8_t VPIN_DIALS_IDX[NUM_SYNTH_DIALS] = {1, 2, 3, 4, 5, 6};

constexpr uint8_t NUM_LED_CHARS = 8 * NUM_LED_PANELS;

char gDesiredLedChars[NUM_LED_CHARS];
char gWrittenLedChars[NUM_LED_CHARS];

AugSynthParam gAugSynthParams[ASP_NUM_PARAMS];

AugSynthDial gSynthDials[NUM_SYNTH_DIALS];

uint8_t gCurrParamPage;

LedControl gLedPanels = LedControl(2,4,3,NUM_LED_PANELS);

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

void AugSynthParam::WriteToScreenBuff(char* buff)
{
    int8_t n = mValue;
    bool negative = n < 0;
    if(negative) n = -n;

    // Write out 2 digit number
    *buff = negative ? '-' : ' ';
    buff += 3;
    *buff-- = ' ';
    *buff-- = '0' + static_cast<char>(n % 10);
    n /= 10;
    *buff = '0' + static_cast<char>(n % 10);
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

void AugSynthDial::UpdateValue(int8_t delta, bool pressed, char* buff)
{
    AugSynthParam* pParam = pressed ? mShiftParamters[gCurrParamPage] : mParamters[gCurrParamPage];

    if(pParam != nullptr)
    {
        pParam->ApplyDelta(delta);
        pParam->WriteToScreenBuff(buff);
    }
}

/// ===================================================================================
/// Public functions
/// ===================================================================================
void InitAugSynth()
{
    InitSynthPatch();
    BindDialsToParams();
    gCurrParamPage = AugSynthPage::ASP_GENERAL;

    for(int i = 0; i < NUM_LED_PANELS; i++)
    {
        delayMicroseconds(10);
        gLedPanels.shutdown(i , false);
        gLedPanels.setIntensity(i, 6);
        gLedPanels.clearDisplay(i);
    }

    for(int i = 0; i < NUM_LED_CHARS; i++)
    {
        gDesiredLedChars[i] = ' ';
        gWrittenLedChars[i] = ' ';
    }
}

#define BIND_ROW(_i_, _page_, _param0_, _param1_, _sparam0_, _sparam1_) \
    gSynthDials[2*_i_].mParamters[AugSynthPage::_page_] = _param0_; \
    gSynthDials[2*_i_].mShiftParamters[AugSynthPage::_page_] = _sparam0_; \
    gSynthDials[2*_i_+1].mParamters[AugSynthPage::_page_] = _param1_; \
    gSynthDials[2*_i_+1].mShiftParamters[AugSynthPage::_page_] = _sparam1_
    
void BindDialsToParams()
{
    // Dials are:
    // 0 1
    // 2 3
    // 4 5

    // General
    BIND_ROW(0, ASP_GENERAL,    &gAugSynthParams[ASP_TUNING],               &gAugSynthParams[ASP_DELAY_MODE], 
                                nullptr,                                    nullptr);

    BIND_ROW(1, ASP_GENERAL,    &gAugSynthParams[ASP_GAIN],                 &gAugSynthParams[ASP_DELAY_TIME],
                                nullptr,                                    &gAugSynthParams[ASP_DELAY_SHEAR]);

    BIND_ROW(2, ASP_GENERAL,    &gAugSynthParams[ASP_DRIVE],                &gAugSynthParams[ASP_DELAY_FEEDBACK],
                                nullptr,                                    nullptr);

    // DCO
    BIND_ROW(0, ASP_DCO,        &gAugSynthParams[ASP_DCO_WAVE_SHAPE_1],     &gAugSynthParams[ASP_DCO_WAVE_SHAPE_2], 
                                &gAugSynthParams[ASP_DCO_PW_1],             &gAugSynthParams[ASP_DCO_PW_2]);

    BIND_ROW(1, ASP_DCO,        &gAugSynthParams[ASP_DCO_TUNE_1],           &gAugSynthParams[ASP_DCO_TUNE_2],
                                &gAugSynthParams[ASP_DCO_PWM_1],            &gAugSynthParams[ASP_DCO_PWM_2]);

    BIND_ROW(2, ASP_DCO,        &gAugSynthParams[ASP_DCO_VOL_1],            &gAugSynthParams[ASP_DCO_VOL_2],
                                nullptr,                                    nullptr);

    // ENV
    BIND_ROW(0, ASP_ENV,        &gAugSynthParams[ASP_ENV_ATTACK1],          &gAugSynthParams[ASP_ENV_ATTACK2], 
                                nullptr,                                    nullptr);

    BIND_ROW(1, ASP_ENV,        &gAugSynthParams[ASP_ENV_SUSTAIN1],         &gAugSynthParams[ASP_ENV_SUSTAIN2],
                                nullptr,                                    nullptr);

    BIND_ROW(2, ASP_ENV,        &gAugSynthParams[ASP_ENV_DECAY1],           &gAugSynthParams[ASP_ENV_DECAY2],
                                &gAugSynthParams[ASP_ENV_RELEASE1],         &gAugSynthParams[ASP_ENV_RELEASE2]);

    // VCF
    BIND_ROW(0, ASP_VCF,        &gAugSynthParams[ASP_VCF_MODE],             &gAugSynthParams[ASP_LFO_GAIN], 
                                nullptr,                                    nullptr);

    BIND_ROW(1, ASP_VCF,        &gAugSynthParams[ASP_VCF_CUTOFF],           &gAugSynthParams[ASP_VCF_CUTOFF_LFO],
                                nullptr,                                    &gAugSynthParams[ASP_LFO_OSC1_VOLUME]);

    BIND_ROW(2, ASP_VCF,        &gAugSynthParams[ASP_VCF_RES],              &gAugSynthParams[ASP_VCF_RES_LFO],
                                nullptr,                                    &gAugSynthParams[ASP_LFO_OSC1_VOLUME]);

    // LFO
    BIND_ROW(0, ASP_LFO,        &gAugSynthParams[ASP_LFO_WAVE_SHAPE],       &gAugSynthParams[ASP_LFO_GAIN], 
                                nullptr,                                    nullptr);

    BIND_ROW(1, ASP_LFO,        &gAugSynthParams[ASP_LFO_RATE],             &gAugSynthParams[ASP_LFO_OSC1_TUNE],
                                nullptr,                                    &gAugSynthParams[ASP_LFO_OSC1_VOLUME]);

    BIND_ROW(2, ASP_LFO,        &gAugSynthParams[ASP_LFO_ATTACK],           &gAugSynthParams[ASP_LFO_OSC2_TUNE],
                                nullptr,                                    &gAugSynthParams[ASP_LFO_OSC2_VOLUME]);
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
    gAugSynthParams[ASP_DELAY_MODE         ] = AugSynthParam(ASP_DELAY_MODE         , 0,  0,   NUM_DELAY_MODES-1);
 
    // DCO 
    gAugSynthParams[ASP_DCO_WAVE_SHAPE_1   ] = AugSynthParam(ASP_DCO_WAVE_SHAPE_1   , 0,  0,   NUM_OSC_MODES-1);
    gAugSynthParams[ASP_DCO_TUNE_1         ] = AugSynthParam(ASP_DCO_TUNE_1         , 0,  -50, 50);
    gAugSynthParams[ASP_DCO_VOL_1          ] = AugSynthParam(ASP_DCO_VOL_1          , 50,  0,  50);
    gAugSynthParams[ASP_DCO_PW_1           ] = AugSynthParam(ASP_DCO_PW_1           , 10, 0,   20);
    gAugSynthParams[ASP_DCO_PWM_1          ] = AugSynthParam(ASP_DCO_PWM_1          , 0,  -20, 20);
    gAugSynthParams[ASP_DCO_WAVE_SHAPE_2   ] = AugSynthParam(ASP_DCO_WAVE_SHAPE_2   , 0,  0,   NUM_OSC_MODES-1);
    gAugSynthParams[ASP_DCO_TUNE_2         ] = AugSynthParam(ASP_DCO_TUNE_2         , 0,  -50, 50);
    gAugSynthParams[ASP_DCO_VOL_2          ] = AugSynthParam(ASP_DCO_VOL_2          , 0,  0,   50);
    gAugSynthParams[ASP_DCO_PW_2           ] = AugSynthParam(ASP_DCO_PW_2           , 10, 0,   20);
    gAugSynthParams[ASP_DCO_PWM_2          ] = AugSynthParam(ASP_DCO_PWM_2          , 0,  -20, 20);

    // ENV
    gAugSynthParams[ASP_ENV_ATTACK1        ] = AugSynthParam(ASP_ENV_ATTACK1        , 0,  5,   99);
    gAugSynthParams[ASP_ENV_DECAY1         ] = AugSynthParam(ASP_ENV_DECAY1         , 0,  10,  99);
    gAugSynthParams[ASP_ENV_SUSTAIN1       ] = AugSynthParam(ASP_ENV_SUSTAIN1       , 0,  40,  50);
    gAugSynthParams[ASP_ENV_RELEASE1       ] = AugSynthParam(ASP_ENV_RELEASE1       , -1, -1,  99); // Note: -1 is decay
    gAugSynthParams[ASP_ENV_ATTACK2        ] = AugSynthParam(ASP_ENV_ATTACK2        , 0,  5,   99);
    gAugSynthParams[ASP_ENV_SUSTAIN2       ] = AugSynthParam(ASP_ENV_SUSTAIN2       , 0,  10,  99);
    gAugSynthParams[ASP_ENV_DECAY2         ] = AugSynthParam(ASP_ENV_DECAY2         , 0,  40,  50);
    gAugSynthParams[ASP_ENV_RELEASE2       ] = AugSynthParam(ASP_ENV_RELEASE2       , -1, -1,  99); // Note -1 is decay
   
    // VCF   
    gAugSynthParams[ASP_VCF_CUTOFF         ] = AugSynthParam(ASP_VCF_CUTOFF         , 0,  0,   50);
    gAugSynthParams[ASP_VCF_RES            ] = AugSynthParam(ASP_VCF_RES            , 0,  0,   20);
    gAugSynthParams[ASP_VCF_MODE           ] = AugSynthParam(ASP_VCF_MODE           , 0,  0,   NUM_FILTER_MODES-1);
    gAugSynthParams[ASP_VCF_CUTOFF_LFO     ] = AugSynthParam(ASP_VCF_CUTOFF_LFO     , 0,  -20, 20);
    gAugSynthParams[ASP_VCF_RES_LFO        ] = AugSynthParam(ASP_VCF_RES_LFO        , 0,  -20, 20);
    gAugSynthParams[ASP_VCF_FOLLOW         ] = AugSynthParam(ASP_VCF_FOLLOW         , 0,  0,   50);
   
    // LFO   
    gAugSynthParams[ASP_LFO_RATE           ] = AugSynthParam(ASP_LFO_RATE           , 25, 0,   99);
    gAugSynthParams[ASP_LFO_WAVE_SHAPE     ] = AugSynthParam(ASP_LFO_WAVE_SHAPE     , 0,  0,   NUM_LFO_OSC_MODES-1);
    gAugSynthParams[ASP_LFO_ATTACK         ] = AugSynthParam(ASP_LFO_ATTACK         , 0,  0,   99);
    gAugSynthParams[ASP_LFO_GAIN           ] = AugSynthParam(ASP_LFO_GAIN           , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC1_TUNE      ] = AugSynthParam(ASP_LFO_OSC1_TUNE      , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC1_VOLUME    ] = AugSynthParam(ASP_LFO_OSC1_VOLUME    , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC2_TUNE      ] = AugSynthParam(ASP_LFO_OSC2_TUNE      , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC1_VOLUME    ] = AugSynthParam(ASP_LFO_OSC1_VOLUME    , 0,  -20, 20);
} 

void UpdateAugSynth()
{
    // Poll encoders
    bool pressed = gVirtualMuxPins[VPIN_PAGE_SELECT_SW].IsActive(); // ~TODO Default preset?
    int8_t pageDelta = gRotaryEncoders[PAGE_SELECT_DIAL_IDX].ConsumeDelta();
    if(pageDelta < 0)
    {
        if(gCurrParamPage > 0)
        {
            gCurrParamPage--;
        }
    }
    else if(pageDelta > 0)
    {
        if(gCurrParamPage < AugSynthPage::NUM_SYNTH_PAGES-1)
        {
            gCurrParamPage++;
        }
    }

    for(uint8_t i = 0; i < NUM_SYNTH_DIALS; i++)
    {
        pressed = gVirtualMuxPins[VPIN_DIALS_SW[i]].IsActive();
        int8_t delta = gRotaryEncoders[VPIN_DIALS_IDX[i]].ConsumeDelta();
        // if(delta != 0)
        // {   
        //     char msgBuff[64];
        //     sprintf(msgBuff, "A %d: %d", (int)i, (int)delta);
		// 	Serial.println(msgBuff);
        // }
        gSynthDials[i].UpdateValue(delta, pressed, (gDesiredLedChars + i * 4));
    }

    uint8_t idx = 0;
    for (uint8_t i = 0; i < NUM_LED_PANELS; i++)
    {
        for(int c = 7; c >= 0; c--)
        {
            char desired = gDesiredLedChars[idx];
            if(gWrittenLedChars[idx] != desired)
            {
                gWrittenLedChars[idx] = desired;
                gLedPanels.setChar(i, c, desired, false);
            }
            idx++;
        }
    }
}