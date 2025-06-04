#ifndef AUG_SYNTH_PARAMS_H
#define AUG_SYNTH_PARAMS_H

// General
#define ASP_TUNING              0 // Int param
#define ASP_DRIVE               1 // 1.0f + gParameters[ASP_DRIVE] * (DRIVE_ALPHA-1.0f);
#define ASP_GAIN                2 // (2.0f / (float_t)MIDI_POLYPHONY) * gParameters[ASP_GAIN];
#define ASP_SOUND_TYPE          39 // Int param

// Delay
#define ASP_DELAY_TIME          3 // gParameters[ASP_DELAY_TIME] * DELAY_BUFFER_LEN
#define ASP_DELAY_FEEDBACK      4 // (uint32_t)(gParameters[ASP_DELAY_FEEDBACK] * 32768.0f);
#define ASP_DELAY_SHEAR         5 // (uint16_t)(gParameters[ASP_DELAY_SHEAR] * (float)AUDIO_BUFF_LEN_DIV4) + 2;
#define ASP_DELAY_MODE          6 // Int Param

// DCO
#define ASP_DCO_WAVE_TYPE_1     7  // Int param
#define ASP_DCO_TUNE_1          8  // powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_1] - 1.5f));
#define ASP_DCO_VOL_1           9  // x*x
#define ASP_DCO_WS_1            10 // 0.5f to 0.95f 
#define ASP_DCO_WAVE_TYPE_2     11 // Int param
#define ASP_DCO_TUNE_2          12 // powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_2] - 1.5f));
#define ASP_DCO_VOL_2           13 // x*x
#define ASP_DCO_WS_2            14 // 0.5f to 0.95f 

// ENV
#define ASP_ENV_ATTACK1         15 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_DECAY1          16 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_SUSTAIN1        17 // 0 to 1
#define ASP_ENV_RELEASE1        18 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_ATTACK2         19 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_DECAY2          20 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_SUSTAIN2        21 // 0 to 1
#define ASP_ENV_RELEASE2        22 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));

// VCF
#define ASP_VCF_CUTOFF          23 // x*x*x
#define ASP_VCF_RES             24 // -0.5 to 0.5
#define ASP_VCF_MODE            25 // Int param
#define ASP_VCF_FOLLOW          26 // -0.5 to 0.5

// LFO
#define ASP_LFO_RATE            27 // (x*x*35+0.1)*SAMPLE_PERIOD
#define ASP_LFO_WAVE_TYPE       28 // Int param
#define ASP_LFO_ATTACK          29 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * atk1));
#define ASP_LFO_WOBBLE          30 // -0.5f to 0.5f
#define ASP_LFO_OSC1_TUNE       31 // -0.5f to 0.5f
#define ASP_LFO_OSC1_VOLUME     32 // -0.5f to 0.5f
#define ASP_LFO_OSC1_SHAPE      33 // -0.5f to 0.5f
#define ASP_LFO_OSC2_TUNE       34 // -0.5f to 0.5f
#define ASP_LFO_OSC2_VOLUME     35 // -0.5f to 0.5f
#define ASP_LFO_OSC2_SHAPE      36 // -0.5f to 0.5f
#define ASP_LFO_VCF_CUTOFF      37 // -0.5 to 0.5
#define ASP_LFO_VCF_RES         38 // -0.5 to 0.5

#define ASP_NUM_PARAMS          40

// Sound types
#define SOUND_TYPE_POLY         0
#define SOUND_TYPE_MONO         1
#define SOUND_TYPE_PIANO        2
#define SOUND_TYPE_BASS         3
#define NUM_SOUND_TYPES         4

// Delay modes
#define DELAY_MODE_OFF          0
#define DELAY_MODE_NORMAL       1
#define DELAY_MODE_SLAPBACK     2
#define DELAY_MODE_GLITCH       3
#define NUM_DELAY_MODES         4

// Oscillator modes
#define OSC_MODE_SINE           0
#define OSC_MODE_SQUARE         1
#define OSC_MODE_SAW            2
#define OSC_MODE_ORGAN          3 
#define NUM_OSC_MODES           4
#define NUM_LFO_OSC_MODES       3

// Filter modes
#define FILTER_MODE_OFF         0
#define FILTER_MODE_LP          1
#define FILTER_MODE_HP          2
#define NUM_FILTER_MODES        3

// Tuning modes
#define TUNING_12TET            0
#define TUNING_Cn_JI            1
#define TUNING_Cs_JI            2
#define TUNING_Dn_JI            3
#define TUNING_Ds_JI            4
#define TUNING_En_JI            5
#define TUNING_Fn_JI            6
#define TUNING_Fs_JI            7
#define TUNING_Gn_JI            8
#define TUNING_Gs_JI            9
#define TUNING_An_JI            10
#define TUNING_As_JI            11
#define TUNING_Bn_JI            12
#define TUNING_24TET            13
#define CIRCLE_OF_5             14
#define WONKY                   15
#define NUM_TUNINGS             16

enum AugSynthPageType
{
    ASP_GENERAL,
    ASP_DELAY,
    ASP_OSC1,
    ASP_OSC2,
    ASP_VCF,
    ASP_LFO,
    ASP_LFO_OSC1,
    ASP_LFO_OSC2,
    ASP_LFO_FILT,
    NUM_PAGE_TYPES
};

#endif // AUG_SYNTH_PARAMS_H