#include "Util/Utils.h"

/// @brief Write a 16-bit value to a 8-bit buffer.
void WriteU16ToByteBuff(uint8_t* buff, uint16_t& idx, uint16_t value)
{
    buff[idx++] = uint8_t(value & 0xFF);        // low byte
    buff[idx++] = uint8_t((value >> 8) & 0xFF); // high byte
}


/// @brief Read a 16-bit value from a 8-bit buffer.
uint16_t ReadU16FromByteBuff(const uint8_t* buff, uint16_t& idx)
{
    uint16_t lo = buff[idx++];
    uint16_t hi = buff[idx++];
    return (hi << 8) | lo;
}