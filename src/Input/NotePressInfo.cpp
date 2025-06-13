#include <Input/NotePressInfo.h>
#include <UserControls.h>

/// ===================================================================================
/// Update
/// ===================================================================================

//-- Change state of note
void NotePressInfo::ChangeState(bool inputPin, uTimeMs time, bool sustain)
{
    if(inputPin) // Note on.
    {
        if(mState != NPS_PRESSED)
        {
            mState = NPS_PRESSED;
            mReleaseTime = time;
        }
    }
    else // Note off.
    {
        if(mState == NPS_PRESSED)
        {
            mReleaseTime = time;
        }
        mState = sustain ? NPS_SUSTAINED : NPS_OFF;
    }
}