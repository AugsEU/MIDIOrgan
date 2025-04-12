#include <TimeInfo.h>

#ifndef NOTE_PRESS_INFO_H
#define NOTE_PRESS_INFO_H

struct NotePressInfo
{
    bool mPressed = false;
    uTimeMs mUpdatedTime = 0;

    void ChangeState(bool inputPin, unsigned long time);
};

#endif // NOTE_PRESS_INFO_H
