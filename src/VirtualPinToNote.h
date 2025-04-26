#include <Arduino.h>

#ifndef VIRTUAL_PIN_TO_NOTE_H
#define VIRTUAL_PIN_TO_NOTE_H

uint8_t VirtualPinToKeyNum(uint8_t vPin);
uint8_t KeyNumToNote(uint8_t vPin, int8_t lowOctave, int8_t upperOctave);
bool IsUpperKey(uint8_t keyNum);

#endif // VIRTUAL_PIN_TO_NOTE_H
