#include <Input/RotaryEncoder.h>

RotaryEncoder::RotaryEncoder() :
    mPrevLeft(false),
    mPrevRight(false),
    mLatch(0),
    mValue(0)
{

}

void RotaryEncoder::UpdateDial(bool left, bool right)
{
    if(!mPrevLeft && !mPrevRight)
    {
        if(left && !right) mLatch = -1;
        else if(right) mLatch = 1;
    }
    if(mLatch != 0)
    {
        if(left && right) // Confirm when 2 waves collide
        {
            if(mLatch < 0)
            {
                if(mValue > -127)
                {
                    mValue--;
                }
            }
            else if(mLatch > 0)
            {
                if(mValue < 127)
                {
                    mValue++;
                }
            }
            mLatch = 0;
        }
    }

    if(!left && !right)
    {
        mLatch = 0;
    }

    mPrevLeft = left;
    mPrevRight = right;
}

int8_t RotaryEncoder::ConsumeDelta()
{
    if(mValue > 0)
    {
        mValue--;
        return 1;
    }
    else if(mValue < 0)
    {
        mValue++;
        return -1;
    }

    return 0;
}