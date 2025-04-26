#include <StableState.h>

constexpr uint8_t ENABLE_THRESH = 32;
constexpr uint8_t STATE_MAX = 64;

/// @brief Send in pin state
void StableState::UpdateState(bool nextState)
{
    if (!nextState && mState > 0)
    {
        mState--;
        if (mState == 0x80)
        {
            // Put it below thresh to deactivate it.
            mState = ENABLE_THRESH / 2;
        }
    }
    else if(nextState && mState < STATE_MAX + 0x80)
    {
        mState++;
        if (mState == ENABLE_THRESH)
        {
            mState = ENABLE_THRESH + 0x80;
        }
    }
}

/// @brief Is this state active?
bool StableState::IsActive()
{
    return mState >= 0x80;
}