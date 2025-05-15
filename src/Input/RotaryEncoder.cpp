#include <Input/RotaryEncoder.h>

int8_t RotaryEncoder::UpdateDial(bool left, bool right)
{
    int8_t delta = 0;

    if(!left && mPrevLeft)
    {
        delta = -1;
    }
    else if(!right && mPrevRight)
    {
        delta = 1;
    }

    mPrevLeft = left;
    mPrevRight = right;

    return delta;
}