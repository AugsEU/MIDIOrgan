#include <Arduino.h>
#include <Util/PortMap.h>

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

#define PORT_ARP_SELECT_UPPER 	PORT_DPIN_50
#define PORT_ARP_SELECT_LOWER 	PORT_DPIN_49
#define PORT_ARP_HOLD 			PORT_DPIN_48
#define PORT_ARP_UP 			PORT_DPIN_47
#define PORT_ARP_DOWN 			PORT_DPIN_39
#define PORT_ARP_SPEC 			PORT_DPIN_42
#define PORT_ARP_FAST 			PORT_DPIN_38
#define PORT_ARP_SLOW 			PORT_DPIN_46

#define PORT_METRONOME 			PORT_DPIN_45
#define PORT_LOOP1 				PORT_DPIN_41
#define PORT_LOOP2 				PORT_DPIN_44
#define PORT_LOOP3 				PORT_DPIN_40
#define PORT_LOOP4 				PORT_DPIN_43

constexpr uint8_t PINA_ARP_GATE = PIN_A15;
constexpr uint8_t PINA_MIDI_CH_UPPER = PIN_A10;
constexpr uint8_t PINA_MIDI_CH_LOWER = PIN_A14;
constexpr uint8_t PINA_OCTAVE_UPPER = PIN_A8;
constexpr uint8_t PINA_OCTAVE_LOWER = PIN_A13;
constexpr uint8_t PINA_TEMPO = PIN_A9;
constexpr uint8_t PINA_KNOB6 = PIN_A12;
constexpr uint8_t PINA_KNOB7 = PIN_A11;

constexpr uint8_t PINA_PEDAL = PIN_A7; // 922 - 9


// Keys
constexpr uint8_t PIN_MUX_START = 22;
constexpr uint8_t NUM_MUX_PIN = 14;
constexpr uint8_t PIN_MUX_S0 = 51;
constexpr uint8_t PIN_MUX_S1 = 52;
constexpr uint8_t PIN_MUX_S2 = 53;

constexpr size_t NUM_VIRTUAL_MUX_PIN = NUM_MUX_PIN * 8; // 8-to-1 mux chips

constexpr size_t NOTES_VPIN_START = 8;
constexpr uint8_t NUM_NOTES = 88;
constexpr uint8_t NOTE_START = 10;

constexpr uint8_t NOTE_NONE = 255;

constexpr int8_t NUM_LOWER_KEYS = 44;
constexpr int8_t NUM_UPPER_KEYS = 44;

// Assert constants make sense
static_assert(NUM_NOTES + NOTES_VPIN_START <= NUM_VIRTUAL_MUX_PIN, 
								"Not enough vpins for notes.");
static_assert((int)NOTE_START + (int)NUM_NOTES < 128, "Note index exceeds midi maximum");
static_assert(NUM_LOWER_KEYS + NUM_UPPER_KEYS == NUM_NOTES);

#endif // CONSTANTS_H
