#include <Arduino.h>
#include <TimeInfo.h>
#include <Arp.h>
#include <StableState.h>
#include <NotePressInfo.h>
#include <MIDI.h>
#include <Constants.h>

#ifndef GLOBALS_H
#define GLOBALS_H

extern uTimeMs gTime;
extern uTimeMs gPrevTime;
extern StableState gVirtualMuxPins[NUM_VIRTUAL_MUX_PIN];
extern NotePressInfo gNoteStates[NUM_NOTES];
extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI;

#endif // GLOBALS_H