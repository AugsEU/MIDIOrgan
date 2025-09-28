#include "Arduino.h"

#ifndef UTILS_H
#define UTILS_H

void WriteU16ToByteBuff(uint8_t* buf, uint16_t& idx, uint16_t value);
uint16_t ReadU16FromByteBuff(const uint8_t* buf, uint16_t& idx);
void WaitForEEPROM(uint8_t timeout = 50);

#endif // UTILS_H