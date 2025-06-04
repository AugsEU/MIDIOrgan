#include <Input/NotePressInfo.h>
#include <UserControls.h>

/// ===================================================================================
/// Update
/// ===================================================================================

//-- Change state of note
void NotePressInfo::ChangeState(bool inputPin, uTimeMs time, bool sustain)
{
    if (time < mPressedTime)
    {
        //mPressedTime = time; // Handle overflow.
    }

    if(inputPin) // Note on.
    {
        if(mState != NPS_PRESSED)
        {
            mState = NPS_PRESSED;
            mPressedTime = time;
        }
    }
    else // Note off.
    {
        mState = sustain ? NPS_SUSTAINED : NPS_OFF;
    }
}