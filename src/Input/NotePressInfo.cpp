#include <Input/NotePressInfo.h>
#include <UserControls.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr int NOTE_OFF_DELAY_TIME_MS = 10;
constexpr int NOTE_ON_DELAY_TIME_MS = 10;


/// ===================================================================================
/// Update
/// ===================================================================================

//-- Change state of note
void NotePressInfo::ChangeState(bool inputPin, uTimeMs time, bool sustain = false)
{
    if (time < mUpdatedTime)
    {
        mUpdatedTime = time; // Handle overflow.
    }

    if (inputPin == mState)
    {
        // Nothing to do here.
        return;
    }
    
    if(!inputPin) // Note off.
    {
        if (time - mUpdatedTime > NOTE_OFF_DELAY_TIME_MS)
        {
            mState = sustain ? NPS_SUSTAINED : NPS_OFF;
            mUpdatedTime = time;
        }
    }
    else if(inputPin) // Note on.
    {
        if (time - mUpdatedTime > NOTE_ON_DELAY_TIME_MS)
        {
            mState = NPS_PRESSED;
            mUpdatedTime = time;
        }
    }
}