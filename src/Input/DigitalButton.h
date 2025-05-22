#include <Util/TimeInfo.h>

#ifndef DIGITAL_BUTTON_H
#define DIGITAL_BUTTON_H

struct DigitalButton
{
    bool mPrevDown = false;
    bool mDown = false;

    void UpdateState(bool down);
    bool IsDown();
    bool IsPressed();
    bool IsReleased();
};

#endif // DIGITAL_BUTTON_H