#include <Arduino.h>

#ifndef MIDI_OUTPUT_H
#define MIDI_OUTPUT_H

void MidiOutputSetup();
void UpdateMidiOutput();

void CancelAllNotes(bool upper);
void SendNoteOn(uint8_t keyNum);
void SendNoteOff(uint8_t keyNum);
void SendNoteOn(uint8_t noteNum, bool upper);
void SendNoteOff(uint8_t noteNum, bool upper);
void SendNoteOnAllCh(uint8_t noteNum);
void SendNoteOffAllCh(uint8_t noteNum);

uint8_t KeyNumToNote(uint8_t keyNum);

void PlayMetronome();



#endif // MIDI_OUTPUT_H