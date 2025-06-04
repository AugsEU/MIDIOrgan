#include <AugSynth.h>
#include <AugSynthParams.h>
#include <UserControls.h>
#include <MidiOutput.h>
#include <Input/DigitalButton.h>
#include <ScreenDisplay.h>

/// ===================================================================================
/// Globals
/// ===================================================================================

const int SAMPLE_RATE = 32000;
const float DRIVE_K = 1.0f;
const float DRIVE_ALPHA = 4.0f * DRIVE_K + 1.0f;
const float ENV_MAX_LENGTH = 30.0f;
const float ENV_MIN_LENGTH = 0.01f;

constexpr uint8_t VPIN_PAGE_SELECT_SW = 6;
constexpr uint8_t PAGE_SELECT_DIAL_IDX = 0;
constexpr uint8_t NUM_SYNTH_PAGES = 11;


constexpr uint8_t VPIN_DIALS_SW[NUM_SYNTH_DIALS] = {1, 0, 3, 5};
constexpr uint8_t VPIN_DIALS_IDX[NUM_SYNTH_DIALS] = {1, 2, 3, 4};

AugSynthParam gAugSynthParams[ASP_NUM_PARAMS];

uint8_t gCurrParamPage = 0;
AugSynthPageParams gSynthPages[NUM_SYNTH_PAGES];
DigitalButton gSynthEditBtn;

uint8_t gForceIdx = 0;

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

// void AugSynthParam::WriteToScreenBuff(char* buff)
// {
//     int8_t n = mValue;
//     bool negative = n < 0;
//     if(negative) n = -n;

//     // Write out 2 digit number
//     *buff = negative ? '-' : ' ';
//     buff += 3;
//     *buff-- = ' ';
//     *buff-- = '0' + static_cast<char>(n % 10);
//     n /= 10;
//     *buff = '0' + static_cast<char>(n % 10);
// }

float AugSynthParam::GetNormFloatValue()
{
    if(mMinValue < 0 && mValue < 0)
    {
        return -(float)mValue / (float)mMinValue;
    }

    return (float)mValue / (float)mMaxValue;
}

float AugSynthParam::ScaleFloat(uint8_t paramNum, float fv)
{
    switch (paramNum)
    {
    // General
    case ASP_DRIVE:
	case ASP_GAIN: // 0 to 1
        fv *= fv;
		break;
	case ASP_DELAY_TIME: // 0 to 1 
    case ASP_DELAY_FEEDBACK:
    case ASP_DELAY_SHEAR:
    case ASP_DELAY_MODE:
		break;

    // DCO
	case ASP_DCO_TUNE_1:
	case ASP_DCO_TUNE_2:
        fv = fv * fv * fv;
        fv = powf(4, fv);
		break;
	case ASP_DCO_VOL_1:
	case ASP_DCO_VOL_2:
        fv *= fv;
		break;
	case ASP_DCO_WS_1: // 0.0f to 1.0f
	case ASP_DCO_WS_2:
		break;

    // ENV
	case ASP_ENV_ATTACK1: // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
    case ASP_ENV_DECAY1:
    case ASP_ENV_RELEASE1:
	case ASP_ENV_ATTACK2:
    case ASP_ENV_DECAY2:
    case ASP_ENV_RELEASE2:
        fv *= fv;// Give weight to small values.
        fv = 1.0f - fv;
        fv = 1.0f / ((float)SAMPLE_RATE * (ENV_MAX_LENGTH + ENV_MIN_LENGTH - ENV_MAX_LENGTH * fv));
		break;
	case ASP_ENV_SUSTAIN1: // 0 to 1 
    case ASP_ENV_SUSTAIN2:
		break;

    // VCF
	case ASP_VCF_CUTOFF: // x*x
        fv = fv * fv;
		break;
	case ASP_VCF_RES: // 0 to 1 weight high values
    case ASP_VCF_FOLLOW:
        fv = 1.0f - fv;
        fv *= fv * fv;
        fv = 1.0f - fv;
		break;

    // LFO
	case ASP_LFO_RATE: // (x*x*50+0.1)*SAMPLE_PERIOD 
        fv *= fv;
        fv *= 50.0f;
        fv += 0.1f;
        fv *= (1.0f / (float)SAMPLE_RATE);
		break;
	case ASP_LFO_ATTACK:
        fv = 1.0f - fv;
        fv = 1.0f / ((float)SAMPLE_RATE * (ENV_MAX_LENGTH + ENV_MIN_LENGTH - ENV_MAX_LENGTH * fv));
		break;
	case ASP_LFO_WOBBLE: // -0.5f to 0.5f
    case ASP_LFO_OSC1_VOLUME:
    case ASP_LFO_OSC2_VOLUME:
    case ASP_LFO_OSC1_SHAPE:
    case ASP_LFO_OSC2_SHAPE:
    case ASP_LFO_VCF_CUTOFF:
    case ASP_LFO_VCF_RES:
        fv *= 0.5f;
		break;
    case ASP_LFO_OSC1_TUNE: // -1 to 1 weighted to small values
    case ASP_LFO_OSC2_TUNE:
        fv *= fabsf(fv);
        break;
    }

    return fv;
}

void AugSynthParam::SendValueToBpSynth()
{       
    uint8_t uv = (uint8_t)mValue;
    switch (mParamNum) // Int params
    {
    case ASP_DELAY_MODE: 
	case ASP_TUNING:
    case ASP_DCO_WAVE_TYPE_1:
    case ASP_DCO_WAVE_TYPE_2:
    case ASP_VCF_MODE:
    case ASP_LFO_WAVE_TYPE:
    case ASP_SOUND_TYPE:
        SendParameterToBp(mParamNum, uv);
		return;
    default:
        break;
    }

    float fv = GetNormFloatValue();
    switch (mParamNum)
    {
    case ASP_DCO_TUNE_1:
	case ASP_DCO_TUNE_2:
        switch (mValue)
        {
        // Special case: values get rounded to exact ratios for better tuning.
        case 46:  fv = 3.0f; break;
        case 40:  fv = 2.0f; break;
        case 33:  fv = 1.5f; break;
        case 30:  fv = 1.3333333333f; break;
        case -30: fv = 0.75f; break;
        case -33: fv = 0.6666666666f; break;
        case -40: fv = 0.5f; break;
        case -46: fv = 0.33333333333f; break;
        default:
            fv = ScaleFloat(mParamNum, fv);
            break;
        }
		break;
    default:
        fv = ScaleFloat(mParamNum, fv);
    }

    SendParameterToBp(mParamNum, fv);
}

/// ===================================================================================
/// AugSynthPageParams
/// ===================================================================================
AugSynthPageParams::AugSynthPageParams()
{
    for(uint8_t i = 0; i < NUM_SYNTH_DIALS; i++)
    {
        mParameters[i] = nullptr;
    }

    mPageType = AugSynthPageType::ASP_GENERAL;
}

void AugSynthPageParams::UpdateValues()
{
    for(uint8_t i = 0; i < NUM_SYNTH_DIALS; i++)
    {
        int8_t delta = gRotaryEncoders[VPIN_DIALS_IDX[i]].ConsumeDelta();
        if(mParameters[i] != nullptr && delta != 0 && mParameters[i] != gpPedalInternalParam)
        {
            mParameters[i]->ApplyDelta(delta);
            mParameters[i]->SendValueToBpSynth();
        }
    }
    
}

/// ===================================================================================
/// Public functions
/// ===================================================================================
void InitAugSynth()
{
    delay(300); // Wait for bp to be online

    InitSynthPatch();
    BindSynthPages();
    gCurrParamPage = 0;
}

#define BIND_SCREEN(_i_, _page_, _param0_, _param1_, _param2_, _param3_) \
    gSynthPages[_i_].mPageType = AugSynthPageType::_page_; \
    gSynthPages[_i_].mParameters[0] = _param0_; \
    gSynthPages[_i_].mParameters[1] = _param1_; \
    gSynthPages[_i_].mParameters[2] = _param2_; \
    gSynthPages[_i_].mParameters[3] = _param3_; \
    static_assert(_i_ < NUM_SYNTH_PAGES)
    
void BindSynthPages()
{
    // Dials are:
    // 0 1
    // 2 3

    // General
    BIND_SCREEN(0, ASP_GENERAL, &gAugSynthParams[ASP_GAIN],                 &gAugSynthParams[ASP_TUNING],
                                &gAugSynthParams[ASP_DRIVE],                &gAugSynthParams[ASP_SOUND_TYPE]);

    // Osc1
    BIND_SCREEN(1, ASP_OSC1,    &gAugSynthParams[ASP_DCO_WAVE_TYPE_1],      &gAugSynthParams[ASP_DCO_VOL_1],
                                &gAugSynthParams[ASP_DCO_WS_1],             &gAugSynthParams[ASP_DCO_TUNE_1]);

    BIND_SCREEN(2, ASP_OSC1,    &gAugSynthParams[ASP_ENV_ATTACK1],          &gAugSynthParams[ASP_ENV_SUSTAIN1],
                                &gAugSynthParams[ASP_ENV_DECAY1],           &gAugSynthParams[ASP_ENV_RELEASE1]);

    // Osc2
    BIND_SCREEN(3, ASP_OSC2,    &gAugSynthParams[ASP_DCO_WAVE_TYPE_2],      &gAugSynthParams[ASP_DCO_VOL_2],
                                &gAugSynthParams[ASP_DCO_WS_2],             &gAugSynthParams[ASP_DCO_TUNE_2]);

    BIND_SCREEN(4, ASP_OSC2,    &gAugSynthParams[ASP_ENV_ATTACK2],          &gAugSynthParams[ASP_ENV_SUSTAIN2],
                                &gAugSynthParams[ASP_ENV_DECAY2],           &gAugSynthParams[ASP_ENV_RELEASE2]);
    
    // VCF
    BIND_SCREEN(5, ASP_VCF,     &gAugSynthParams[ASP_VCF_MODE],             &gAugSynthParams[ASP_VCF_CUTOFF],
                                &gAugSynthParams[ASP_VCF_FOLLOW],           &gAugSynthParams[ASP_VCF_RES]);

    // Lfo
    BIND_SCREEN(6, ASP_LFO,     &gAugSynthParams[ASP_LFO_WAVE_TYPE],        &gAugSynthParams[ASP_LFO_ATTACK],
                                &gAugSynthParams[ASP_LFO_RATE],             &gAugSynthParams[ASP_LFO_WOBBLE]);

    BIND_SCREEN(7, ASP_LFO_OSC1,&gAugSynthParams[ASP_LFO_OSC1_VOLUME],      &gAugSynthParams[ASP_LFO_OSC1_SHAPE],
                                &gAugSynthParams[ASP_LFO_OSC1_TUNE],        nullptr);
    
    BIND_SCREEN(8, ASP_LFO_OSC2,&gAugSynthParams[ASP_LFO_OSC2_VOLUME],      &gAugSynthParams[ASP_LFO_OSC2_SHAPE],
                                &gAugSynthParams[ASP_LFO_OSC2_TUNE],        nullptr);

    BIND_SCREEN(9, ASP_LFO_FILT,&gAugSynthParams[ASP_LFO_VCF_CUTOFF],       nullptr,
                                &gAugSynthParams[ASP_LFO_VCF_RES],          nullptr);

    // Delay
    BIND_SCREEN(10, ASP_DELAY,  &gAugSynthParams[ASP_DELAY_MODE],           &gAugSynthParams[ASP_DELAY_FEEDBACK],
                                &gAugSynthParams[ASP_DELAY_TIME],           &gAugSynthParams[ASP_DELAY_SHEAR]);
}

void InitSynthPatch()
{
    // General          Type                                                     Default  min  max
    gAugSynthParams[ASP_TUNING             ] = AugSynthParam(ASP_TUNING             , 0,  0,   NUM_TUNINGS-1);
    gAugSynthParams[ASP_DRIVE              ] = AugSynthParam(ASP_DRIVE              , 0,  0,   50);
    gAugSynthParams[ASP_GAIN               ] = AugSynthParam(ASP_GAIN               , 25, 0,   50);
    gAugSynthParams[ASP_DELAY_TIME         ] = AugSynthParam(ASP_DELAY_TIME         , 0,  0,   50);
    gAugSynthParams[ASP_DELAY_FEEDBACK     ] = AugSynthParam(ASP_DELAY_FEEDBACK     , 0,  0,   50);
    gAugSynthParams[ASP_DELAY_SHEAR        ] = AugSynthParam(ASP_DELAY_SHEAR        , 0,  0,   50);
    gAugSynthParams[ASP_DELAY_MODE         ] = AugSynthParam(ASP_DELAY_MODE         , 0,  0,   NUM_DELAY_MODES-1);
    gAugSynthParams[ASP_SOUND_TYPE         ] = AugSynthParam(ASP_SOUND_TYPE         , 0,  0,   NUM_SOUND_TYPES-1);
 
    // DCO 
    gAugSynthParams[ASP_DCO_WAVE_TYPE_1    ] = AugSynthParam(ASP_DCO_WAVE_TYPE_1    , 0,  0,   NUM_OSC_MODES-1);
    gAugSynthParams[ASP_DCO_TUNE_1         ] = AugSynthParam(ASP_DCO_TUNE_1         , 0,  -50, 50);
    gAugSynthParams[ASP_DCO_VOL_1          ] = AugSynthParam(ASP_DCO_VOL_1          , 50, 0,   50);
    gAugSynthParams[ASP_DCO_WS_1           ] = AugSynthParam(ASP_DCO_WS_1           , 5,  0,   20);
    gAugSynthParams[ASP_DCO_WAVE_TYPE_2    ] = AugSynthParam(ASP_DCO_WAVE_TYPE_2    , 0,  0,   NUM_OSC_MODES-1);
    gAugSynthParams[ASP_DCO_TUNE_2         ] = AugSynthParam(ASP_DCO_TUNE_2         , 0,  -50, 50);
    gAugSynthParams[ASP_DCO_VOL_2          ] = AugSynthParam(ASP_DCO_VOL_2          , 0,  0,   50);
    gAugSynthParams[ASP_DCO_WS_2           ] = AugSynthParam(ASP_DCO_WS_2           , 5,  0,   20);

    // ENV
    gAugSynthParams[ASP_ENV_ATTACK1        ] = AugSynthParam(ASP_ENV_ATTACK1        , 5,  0,   99);
    gAugSynthParams[ASP_ENV_DECAY1         ] = AugSynthParam(ASP_ENV_DECAY1         , 10, 0,   99);
    gAugSynthParams[ASP_ENV_SUSTAIN1       ] = AugSynthParam(ASP_ENV_SUSTAIN1       , 40, 0,   50);
    gAugSynthParams[ASP_ENV_RELEASE1       ] = AugSynthParam(ASP_ENV_RELEASE1       , 10, 0,   99);
    gAugSynthParams[ASP_ENV_ATTACK2        ] = AugSynthParam(ASP_ENV_ATTACK2        , 5,  0,   99);
    gAugSynthParams[ASP_ENV_DECAY2         ] = AugSynthParam(ASP_ENV_DECAY2         , 10, 0,   99);
    gAugSynthParams[ASP_ENV_SUSTAIN2       ] = AugSynthParam(ASP_ENV_SUSTAIN2       , 40, 0,   50);
    gAugSynthParams[ASP_ENV_RELEASE2       ] = AugSynthParam(ASP_ENV_RELEASE2       , 10, 0,   99);
   
    // VCF   
    gAugSynthParams[ASP_VCF_CUTOFF         ] = AugSynthParam(ASP_VCF_CUTOFF         , 0,  0,   50);
    gAugSynthParams[ASP_VCF_RES            ] = AugSynthParam(ASP_VCF_RES            , 0,  0,   20);
    gAugSynthParams[ASP_VCF_MODE           ] = AugSynthParam(ASP_VCF_MODE           , 0,  0,   NUM_FILTER_MODES-1);
    gAugSynthParams[ASP_VCF_FOLLOW         ] = AugSynthParam(ASP_VCF_FOLLOW         , 0,  0,   20);
   
    // LFO   
    gAugSynthParams[ASP_LFO_RATE           ] = AugSynthParam(ASP_LFO_RATE           , 25, 0,   99);
    gAugSynthParams[ASP_LFO_WAVE_TYPE      ] = AugSynthParam(ASP_LFO_WAVE_TYPE      , 0,  0,   NUM_LFO_OSC_MODES-1);
    gAugSynthParams[ASP_LFO_ATTACK         ] = AugSynthParam(ASP_LFO_ATTACK         , 0,  0,   99);
    gAugSynthParams[ASP_LFO_WOBBLE         ] = AugSynthParam(ASP_LFO_WOBBLE         , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC1_TUNE      ] = AugSynthParam(ASP_LFO_OSC1_TUNE      , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC1_VOLUME    ] = AugSynthParam(ASP_LFO_OSC1_VOLUME    , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC1_SHAPE     ] = AugSynthParam(ASP_LFO_OSC1_SHAPE     , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC2_TUNE      ] = AugSynthParam(ASP_LFO_OSC2_TUNE      , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC2_VOLUME    ] = AugSynthParam(ASP_LFO_OSC2_VOLUME    , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_OSC2_SHAPE     ] = AugSynthParam(ASP_LFO_OSC2_SHAPE     , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_VCF_CUTOFF     ] = AugSynthParam(ASP_LFO_VCF_CUTOFF     , 0,  -20, 20);
    gAugSynthParams[ASP_LFO_VCF_RES        ] = AugSynthParam(ASP_LFO_VCF_RES        , 0,  -20, 20);

    SendAllParams();
} 

void SendAllParams()
{
    for(uint8_t i = 0; i < ASP_NUM_PARAMS; i++)
    {
        gAugSynthParams->SendValueToBpSynth();
    }
}

void UpdateAugSynth()
{
    ScreenPage currPage = gCurrScreenPage;
    bool onSynthEdit = currPage == ScreenPage::SP_AUG_SYNTH_EDIT;
 
    // Poll encoders
    bool pressed = gVirtualMuxPins[VPIN_PAGE_SELECT_SW].IsActive();
    gSynthEditBtn.UpdateState(pressed);
    if(gSynthEditBtn.IsPressed())
    {
        if(!onSynthEdit)
        {
            SetScreenPage(ScreenPage::SP_AUG_SYNTH_EDIT);
        }
        else
        {
            SetScreenPage(ScreenPage::SP_GENERAL_INFO);
        }
    }

    if(onSynthEdit)
    {
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
            if(gCurrParamPage < NUM_SYNTH_PAGES-1)
            {
                gCurrParamPage++;
            }
        }

        GetCurrPageParams().UpdateValues();
    }

    AugSynthParam* pForceParam = &gAugSynthParams[gForceIdx++];
    if(pForceParam != gpPedalInternalParam)
    {
        pForceParam->SendValueToBpSynth();
    }
    if(gForceIdx >= ASP_NUM_PARAMS)
    {
        gForceIdx = 0;
    }
}


AugSynthPageParams& GetCurrPageParams()
{
    return gSynthPages[gCurrParamPage];
}