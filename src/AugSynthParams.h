#ifndef AUG_SYNTH_PARAMS_H
#define AUG_SYNTH_PARAMS_H

// General
#define ASP_TUNING              0
#define ASP_DRIVE               1
#define ASP_GAIN                2
#define ASP_DELAY_TIME          3
#define ASP_DELAY_FEEDBACK      4
#define ASP_DELAY_SHEAR         5
#define ASP_DELAY_MODE          6

// DCO
#define ASP_DCO_WAVE_SHAPE_1    7
#define ASP_DCO_TUNE_1          8
#define ASP_DCO_VOL_1           9
#define ASP_DCO_PW_1            10
#define ASP_DCO_PWM_1           11
#define ASP_DCO_WAVE_SHAPE_2    12
#define ASP_DCO_TUNE_2          13
#define ASP_DCO_VOL_2           14
#define ASP_DCO_PW_1            15
#define ASP_DCO_PWM_1           16

// ENV
#define ASP_ENV_ATTACK1         17
#define ASP_ENV_DECAY1          18
#define ASP_ENV_SUSTAIN1        19
#define ASP_ENV_RELEASE1        20
#define ASP_ENV_ATTACK2         21
#define ASP_ENV_SUSTAIN2        22
#define ASP_ENV_DECAY2          23
#define ASP_ENV_RELEASE2        24

// VCF
#define ASP_VCF_CUTOFF          25
#define ASP_VCF_RES             26
#define ASP_VCF_MODE            27
#define ASP_VCF_CUTOFF_LFO      28
#define ASP_VCF_RES_LFO         29
#define ASP_VCF_FOLLOW          30

// LFO
#define ASP_LFO_RATE            31
#define ASP_LFO_WAVE_SHAPE      32
#define ASP_LFO_ATTACK          33
#define ASP_LFO_GAIN            34
#define ASP_LFO_OSC1_TUNE       35
#define ASP_LFO_OSC1_VOLUME     36
#define ASP_LFO_OSC2_TUNE       37
#define ASP_LFO_OSC1_VOLUME     38

#define ASP_NUM_PARAMS          39

enum AugSynthValueCategory
{
    General,
    DCO,
    VCF,
    LFO,
    ENV
};

#endif // AUG_SYNTH_PARAMS_H