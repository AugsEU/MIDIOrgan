#include <Arduino.h>

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

struct RotaryEncoder
{
    bool mPrevLeft : 1;
    bool mPrevRight : 1;
    int mLatch : 4;

    int8_t mValue;

    RotaryEncoder();

    void UpdateDial(bool left, bool right);
    int8_t ConsumeDelta();
};

#endif // ROTARY_ENCODER_H