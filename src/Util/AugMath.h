#include <Arduino.h>

#ifndef AUG_MATH_H
#define AUG_MATH_H

template<typename T>
inline T RoundToNearestMultiple(T i, T n)
{   
    T r = i % n;
    return (r > n/2) ? (i + n - r) : (i - r);
}

inline uint8_t ApplyDelta(uint8_t t, int8_t d, uint8_t maxT)
{
    if(d<0 && t<(uint8_t)(-d))
    {
        return 0;
    }

    // assumes t < 128
    t = (uint8_t)((int8_t)t+d);
    return min(t, maxT);
}

#endif // AUG_MATH_H