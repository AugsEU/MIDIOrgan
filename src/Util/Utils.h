#include "Arduino.h"

#ifndef UTILS_H
#define UTILS_H

void WriteU16ToByteBuff(uint8_t* buf, uint16_t& idx, uint16_t value);
uint16_t ReadU16FromByteBuff(const uint8_t* buf, uint16_t& idx);

#endif // UTILS_H