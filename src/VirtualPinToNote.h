#include <Arduino.h>

#ifndef VIRTUAL_PIN_TO_NOTE_H
#define VIRTUAL_PIN_TO_NOTE_H

uint8_t VirtualPinToNote(uint8_t vPin, int8_t lowOctave = 2, int8_t upperOctave = 0);

#endif // VIRTUAL_PIN_TO_NOTE_H
