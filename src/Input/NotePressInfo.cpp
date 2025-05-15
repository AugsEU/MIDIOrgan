#include <Input/NotePressInfo.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr int NOTE_OFF_DELAY_TIME_MS = 40;
constexpr int NOTE_ON_DELAY_TIME_MS = 40;


/// ===================================================================================
/// Update
/// ===================================================================================

//-- Change state of note
void NotePressInfo::ChangeState(bool inputPin, uTimeMs time)
{
    if (time < mUpdatedTime)
    {
        mUpdatedTime = time; // Handle overflow.
    }

    if (inputPin == mPressed)
    {
        // Nothing to do here.
        return;
    }
    
    if(!inputPin) // Note off.
    {
        if (time - mUpdatedTime > NOTE_OFF_DELAY_TIME_MS)
        {
            mPressed = inputPin;
            mUpdatedTime = time;
        }
    }
    else if(inputPin) // Note on.
    {
        if (time - mUpdatedTime > NOTE_ON_DELAY_TIME_MS)
        {
            mPressed = inputPin;
            mUpdatedTime = time;
        }
    }
}