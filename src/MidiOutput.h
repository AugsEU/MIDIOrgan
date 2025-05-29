#include <Arduino.h>
#include <UserControls.h>
#include <Util/Globals.h>

#define AUG_SYNTH_DEBUG 0

#ifndef MIDI_OUTPUT_H
#define MIDI_OUTPUT_H

enum PedalMode : uint8_t
{
    PM_OFF,
    PM_VELOCITY,
    PM_PITCH_BEND,
    PM_MODULATION,
    PM_VOLUME,
    PM_INTERNAL,

    NUM_PEDAL_MODES
};

enum PedalMidiCh : uint8_t
{
    PMC_UPPER_LOWER,
    PMC_LOWER,
    PMC_UPPER,
    PMC_MIDI_CH1,
    PMC_MIDI_CH2,
    PMC_MIDI_CH3,
    PMC_MIDI_CH4,
    PMC_MIDI_CH5,
    PMC_MIDI_CH6,
    PMC_MIDI_CH7,
    PMC_MIDI_CH8,
    PMC_MIDI_CH9,
    PMC_MIDI_CH10,
    PMC_MIDI_CH11,
    PMC_MIDI_CH12,
    PMC_MIDI_CH13,
    PMC_MIDI_CH14,
    PMC_MIDI_CH15,
    PMC_MIDI_CH16,

    NUM_PEDAL_MIDI_CH
};


enum PedalInternalParam : uint8_t
{
    PIP_GAIN,
    PIP_DELAY_TIME,
    PIP_DELAY_FEEDBACK,
    PIP_DCO_VOL_1,
    PIP_DCO_VOL_2,
    PIP_CUTOFF,
    PIP_LFO_RATE,
    PIP_LFO_OSC1_FREQ,
    PIP_LFO_OSC2_FREQ,

    NUM_PEDAL_INTERNAL_PARAMS
};

extern NotePressInfo gNoteStates[NUM_NOTES];
extern AnalogSelector<uint8_t, 17, 0> gUpperCh;
extern AnalogSelector<uint8_t, 17, 0> gLowerCh;
extern AnalogSelector<int8_t, 5, -3> gUpperOct;
extern AnalogSelector<int8_t, 5, 1> gLowerOct;

#if AUG_SYNTH_DEBUG
extern AnalogSelector<uint8_t, 5, 0> gCatSelector;
#endif // AUG_SYNTH_DEBUG

void MidiOutputSetup();
void UpdateMidiOutput();
PedalMode GetPedalMode();
PedalMidiCh GetPedalMidiCh();
PedalInternalParam GetPedalInternalParam();

void PlayNotesDirect(uint8_t keyStart, uint8_t keyEnd);

void CancelAllNotes(bool upper);

void SendNoteOn(uint8_t keyNum);
void SendNoteOff(uint8_t keyNum);

void SendNoteOn(uint8_t keyNum, uint8_t ch);
void SendNoteOff(uint8_t keyNum, uint8_t ch);

void SendNoteOnAllCh(uint8_t noteNum);
void SendNoteOffAllCh(uint8_t noteNum);

uint8_t KeyNumToNote(uint8_t keyNum);

void PlayMetronome();

bool SendPedalMidiCC(PedalMode mode, uint8_t value, uint8_t ch);
bool SendPedalMidiCC(PedalMode mode, uint8_t value);
bool SendPedalPitchBend(int value, uint8_t ch);
bool SendPedalPitchBend(int value);
bool ChannelIsPedal(uint8_t ch);
void UpdatePedal();

void SendParameterToBp(const uint8_t paramNum, const float value);
void SendParameterToBp(const uint8_t paramNum, const uint8_t value);


#endif // MIDI_OUTPUT_H