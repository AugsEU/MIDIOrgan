#include <Arduino.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

/// ===================================================================================
/// Constants
/// ===================================================================================

constexpr uint8_t PIN_MUX_START = 23;
constexpr uint8_t NUM_MUX_PIN = 11;
constexpr uint8_t PIN_MUX_S0 = 51;
constexpr uint8_t PIN_MUX_S1 = 52;
constexpr uint8_t PIN_MUX_S2 = 53;

constexpr size_t NUM_VIRTUAL_MUX_PIN = NUM_MUX_PIN * 8; // 8-to-1 mux chips

constexpr size_t NOTES_VPIN_START = 0;
constexpr uint8_t NUM_NOTES = 88;
constexpr uint8_t NOTE_START = 10;

constexpr uint16_t DEFAULT_TEMPO = 150;

// Assert constants make sense
static_assert(NUM_NOTES + NOTES_VPIN_START <= NUM_VIRTUAL_MUX_PIN, 
								"Not enough vpins for notes.");
static_assert((int)NOTE_START + (int)NUM_NOTES < 128, "Note index exceeds midi maximum");

#endif // CONSTANTS_H
