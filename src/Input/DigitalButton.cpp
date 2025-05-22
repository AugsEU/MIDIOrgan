#include <Input/DigitalButton.h>

void DigitalButton::UpdateState(bool down)
{
    mPrevDown = mDown;
    mDown = down;
}

bool DigitalButton::IsDown()
{
    return mDown;
}

bool DigitalButton::IsPressed()
{
    return mDown && !mPrevDown;
}

bool DigitalButton::IsReleased()
{
    return !mDown && mPrevDown;
}