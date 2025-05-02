#include <Arduino.h>
#include <UserControls.h>
#include <Globals.h>

#define AUG_SYNTH_DEBUG 1

#ifndef MIDI_OUTPUT_H
#define MIDI_OUTPUT_H

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



#endif // MIDI_OUTPUT_H