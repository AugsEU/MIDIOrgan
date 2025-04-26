#include <Arduino.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

/// ===================================================================================
/// Constants
/// ===================================================================================

// Top panel
constexpr uint8_t PIN_ARP_SELECT_UPPER = 50;
constexpr uint8_t PIN_ARP_SELECT_LOWER = 49;
constexpr uint8_t PIN_ARP_HOLD = 48;
constexpr uint8_t PIN_ARP_UP = 47;
constexpr uint8_t PIN_ARP_DOWN = 39;
constexpr uint8_t PIN_ARP_SPEC = 42;
constexpr uint8_t PIN_ARP_FAST = 38;
constexpr uint8_t PIN_ARP_SLOW = 46;

constexpr uint8_t PIN_METRONOME = 45;
constexpr uint8_t PIN_LOOP1 = 41;
constexpr uint8_t PIN_LOOP2 = 44;
constexpr uint8_t PIN_LOOP3 = 40;
constexpr uint8_t PIN_LOOP4 = 43;

constexpr uint8_t PINA_ARP_GATE = PIN_A15;
constexpr uint8_t PINA_MIDI_CH_UPPER = PIN_A13;
constexpr uint8_t PINA_MIDI_CH_LOWER = PIN_A9;
constexpr uint8_t PINA_OCTAVE_UPPER = PIN_A14;
constexpr uint8_t PINA_OCTAVE_LOWER = PIN_A8;
constexpr uint8_t PINA_TEMPO = PIN_A10;
constexpr uint8_t PINA_KNOB6 = PIN_A12;
constexpr uint8_t PINA_KNOB7 = PIN_A11;


// Keys
constexpr uint8_t PIN_MUX_START = 23;
constexpr uint8_t NUM_MUX_PIN = 11;
constexpr uint8_t PIN_MUX_S0 = 51;
constexpr uint8_t PIN_MUX_S1 = 52;
constexpr uint8_t PIN_MUX_S2 = 53;

constexpr size_t NUM_VIRTUAL_MUX_PIN = NUM_MUX_PIN * 8; // 8-to-1 mux chips

constexpr size_t NOTES_VPIN_START = 0;
constexpr uint8_t NUM_NOTES = 88;
constexpr uint8_t NOTE_START = 10;

// Assert constants make sense
static_assert(NUM_NOTES + NOTES_VPIN_START <= NUM_VIRTUAL_MUX_PIN, 
								"Not enough vpins for notes.");
static_assert((int)NOTE_START + (int)NUM_NOTES < 128, "Note index exceeds midi maximum");

#endif // CONSTANTS_H
