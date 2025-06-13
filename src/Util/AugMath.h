#ifndef AUG_MATH_H
#define AUG_MATH_H

template<typename T>
inline T RoundToNearestMultiple(T i, T n)
{   
    T r = i % n;
    return (r > n/2) ? (i + n - r) : (i - r);
}

#endif // AUG_MATH_H