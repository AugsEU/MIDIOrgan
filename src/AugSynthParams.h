#ifndef AUG_SYNTH_PARAMS_H
#define AUG_SYNTH_PARAMS_H

// General
#define ASP_TUNING              0
#define ASP_DRIVE               1
#define ASP_GAIN                2
#define ASP_DELAY_TIME          3
#define ASP_DELAY_FEEDBACK      4
#define ASP_GENERAL_UNUSED      5

// DCO
#define ASP_DCO_WAVE_SHAPE_1    6
#define ASP_DCO_TUNE_1          7
#define ASP_DCO_VOL_1           8
#define ASP_DCO_WAVE_SHAPE_2    9
#define ASP_DCO_TUNE_2          10
#define ASP_DCO_VOL_2           11 

// VCF
#define ASP_VCF_CUTOFF          12
#define ASP_VCF_RES             13
#define ASP_VCF_MODE            14
#define ASP_VCF_CUTOFF_LFO      15
#define ASP_VCF_RES_LFO         16
#define ASP_VCF_ENV1            17

// LFO
#define ASP_LFO_RATE            18
#define ASP_LFO_WAVE_SHAPE      19
#define ASP_LFO_ATTACK          20
#define ASP_LFO_GAIN            21
#define ASP_LFO_OSC1_TUNE       22
#define ASP_LFO_OSC2_TUNE       23

// ENV
#define ASP_ENV_ATTACK1         24
#define ASP_ENV_SUSTAIN1        25
#define ASP_ENV_DECAY1          26
#define ASP_ENV_ATTACK2         27
#define ASP_ENV_SUSTAIN2        28
#define ASP_ENV_DECAY2          29

#define ASP_NUM_PARAMS          30

enum AugSynthValueCategory
{
    General,
    DCO,
    VCF,
    LFO,
    ENV
};

#endif // AUG_SYNTH_PARAMS_H