#include <Arduino.h>

#ifndef STABLE_STATE_H
#define STABLE_STATE_H

// Represents on on/off state that is stable to faulty connections.
struct StableState
{
    uint8_t mState = 0;

    void UpdateState(bool nextState);
    bool IsActive();
};

#endif // STABLE_STATE_H