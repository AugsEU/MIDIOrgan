#ifndef AUG_SYNTH_PARAMS_H
#define AUG_SYNTH_PARAMS_H

// General
#define ASP_TUNING              0 // Int param
#define ASP_DRIVE               1 // 1.0f + gParameters[ASP_DRIVE] * (DRIVE_ALPHA-1.0f);
#define ASP_GAIN                2 // (2.0f / (float_t)MIDI_POLYPHONY) * gParameters[ASP_GAIN];
#define ASP_DELAY_TIME          3 // gParameters[ASP_DELAY_TIME] * DELAY_BUFFER_LEN
#define ASP_DELAY_FEEDBACK      4 // (uint32_t)(gParameters[ASP_DELAY_FEEDBACK] * 32768.0f);
#define ASP_DELAY_SHEAR         5 // (uint16_t)(gParameters[ASP_DELAY_SHEAR] * (float)AUDIO_BUFF_LEN_DIV4) + 2;
#define ASP_DELAY_MODE          6 // Int Param

// DCO
#define ASP_DCO_WAVE_SHAPE_1    7  // Int param
#define ASP_DCO_TUNE_1          8  // powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_1] - 1.5f));
#define ASP_DCO_VOL_1           9  // x*x
#define ASP_DCO_PW_1            10 // 0.5f to 0.95f 
#define ASP_DCO_PWM_1           11 // -0.5f to 0.5f
#define ASP_DCO_WAVE_SHAPE_2    12 // Int param
#define ASP_DCO_TUNE_2          13 // powf(2, floorf(4.0f * gParameters[ASP_DCO_TUNE_2] - 1.5f));
#define ASP_DCO_VOL_2           14 // x*x
#define ASP_DCO_PW_2            15 // 0.5f to 0.95f 
#define ASP_DCO_PWM_2           16 // -0.5f to 0.5f

// ENV
#define ASP_ENV_ATTACK1         17 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_DECAY1          18 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_SUSTAIN1        19 // 0 to 1
#define ASP_ENV_RELEASE1        20 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_ATTACK2         21 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_SUSTAIN2        22 // 0 to 1
#define ASP_ENV_DECAY2          23 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));
#define ASP_ENV_RELEASE2        24 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * n));

// VCF
#define ASP_VCF_CUTOFF          25 // x*x*x
#define ASP_VCF_RES             26 // -0.5 to 0.5
#define ASP_VCF_MODE            27 // Int param
#define ASP_VCF_CUTOFF_LFO      28 // -0.5 to 0.5
#define ASP_VCF_RES_LFO         29 // -0.5 to 0.5
#define ASP_VCF_FOLLOW          30 // -0.5 to 0.5

// LFO
#define ASP_LFO_RATE            31 // (x*x*35+0.1)*SAMPLE_PERIOD
#define ASP_LFO_WAVE_SHAPE      32 // Int param
#define ASP_LFO_ATTACK          33 // 1.0f / (SAMPLE_RATE * (8.01f - 8.0f * atk1));
#define ASP_LFO_WOBBLE          34 // -0.5f to 0.5f
#define ASP_LFO_OSC1_TUNE       35 // -0.5f to 0.5f
#define ASP_LFO_OSC1_VOLUME     36 // -0.5f to 0.5f
#define ASP_LFO_OSC2_TUNE       37 // -0.5f to 0.5f
#define ASP_LFO_OSC2_VOLUME     38 // -0.5f to 0.5f

#define ASP_NUM_PARAMS          39

// Delay modes
#define DELAY_MODE_NORMAL       0
#define DELAY_MODE_SLAPBACK     1
#define DELAY_MODE_OCTAVE       2
#define DELAY_MODE_REVERSE      3
#define NUM_DELAY_MODES         4

// Oscillator modes
#define OSC_MODE_SINE           0 // Si
#define OSC_MODE_SQUARE         1 // Sq
#define OSC_MODE_SAW            2 // So
#define OSC_MODE_TRIANGLE       3 // tr
#define OSC_MODE_RAMP           4 // ro
#define OSC_MODE_GLITCH         5 // GL
#define OSC_MODE_NOISE          6 // no
#define NUM_OSC_MODES           7
#define NUM_LFO_OSC_MODES       3

// Filter modes
#define FILTER_MODE_OFF         0
#define FILTER_MODE_LP          1
#define FILTER_MODE_BP          2
#define FILTER_MODE_HP          3
#define NUM_FILTER_MODES        4

enum AugSynthPage
{
    ASP_GENERAL,
    ASP_DCO,
    ASP_ENV,
    ASP_VCF,
    ASP_LFO,
    NUM_SYNTH_PAGES
};

#endif // AUG_SYNTH_PARAMS_H