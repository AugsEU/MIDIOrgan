#include <Arduino.h>

#ifndef STABLE_STATE_H
#define STABLE_STATE_H

// Represents on on/off state that is stable to faulty connections.
template <uint8_t thresh>
struct StableState
{
    uint8_t mState = 0;

    void UpdateState(bool nextState)
    {
        if (!nextState && mState > 0)
        {
            mState--;
            if (mState == 0x80)
            {
                // Put it below thresh to deactivate it.
                mState = thresh / 2;
            }
        }
        else if(nextState && mState < thresh*2 + 0x80)
        {
            mState++;
            if (mState == thresh)
            {
                mState = thresh + 0x80;
            }
        }
    }

    bool IsActive()
    {
        return mState >= 0x80;
    }
};

#endif // STABLE_STATE_H