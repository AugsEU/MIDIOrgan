#include <Arduino.h>

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

struct RotaryEncoder
{
    bool mPrevLeft = false;
    bool mPrevRight = false;

    int8_t UpdateDial(bool left, bool right);
};

#endif // ROTARY_ENCODER_H