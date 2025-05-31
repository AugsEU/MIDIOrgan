#include <Util/TimeInfo.h>

#ifndef NOTE_PRESS_INFO_H
#define NOTE_PRESS_INFO_H

enum NotePressState
{
    NPS_OFF,      // Note is not ringing
    NPS_PRESSED,  // Note is pressed
    NPS_SUSTAINED // Note is not pressed but is ringing because of 
};

struct NotePressInfo
{
    NotePressState mState = NPS_OFF;
    uTimeMs mUpdatedTime = 0;

    void ChangeState(bool inputPin, unsigned long time, bool sustain = false);
};

#endif // NOTE_PRESS_INFO_H
