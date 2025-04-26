#include <Arduino.h>

#ifndef MIDI_OUTPUT_H
#define MIDI_OUTPUT_H

void MidiOutputSetup();
void MidiOutputReadPins();

void SendNoteOn(uint8_t keyNum);
void SendNoteOff(uint8_t keyNum);
void SendNoteOn(uint8_t noteNum, bool upper);
void SendNoteOff(uint8_t noteNum, bool upper);

void PlayMetronome();

#endif // MIDI_OUTPUT_H