#ifndef AUG_SYNTH_PARAMS_H
#define AUG_SYNTH_PARAMS_H

#define ASP_TUNING              0
#define ASP_DRIVE               1
#define ASP_GAIN                2

// DCO
#define ASP_DCO_WAVE_SHAPE_1    3
#define ASP_DCO_TUNE_1          4
#define ASP_DCO_VOL_1           5
#define ASP_DCO_WAVE_SHAPE_2    6
#define ASP_DCO_TUNE_2          7
#define ASP_DCO_VOL_2           8  

// VCF
#define ASP_VCF_CUTOFF          9
#define ASP_VCF_RES             10
#define ASP_VCF_MODE            11
#define ASP_VCF_CUTOFF_LFO      12
#define ASP_VCF_RES_LFO         13
#define ASP_VCF_ENV1            14

// LFO
#define ASP_LFO_RATE            15
#define ASP_LFO_WAVE_SHAPE      16
#define ASP_LFO_ATTACK          17
#define ASP_LFO_GAIN            18
#define ASP_LFO_OSC1_TUNE       19
#define ASP_LFO_OSC2_TUNE       20

// ENV
#define ASP_ENV_ATTACK1         21
#define ASP_ENV_SUSTAIN1        22
#define ASP_ENV_DECAY1          23
#define ASP_ENV_ATTACK2         24
#define ASP_ENV_SUSTAIN2        25
#define ASP_ENV_DECAY2          26

#define ASP_NUM_PARAMS          27

enum AugSynthValueCategory
{
    General,
    DCO,
    VCF,
    LFO,
    ENV
};

#endif // AUG_SYNTH_PARAMS_H