#include <Arduino.h>

#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

template <typename T, size_t MaxLength>
class FixedArray
{
private:
    T mData[MaxLength];
    size_t mCurrentLength = 0;

public:
    // Standard array access functions
    T* begin() { return mData; }
    const T* begin() const { return mData; }
    T* end() { return mData + mCurrentLength; }
    const T* end() const { return mData + mCurrentLength; }
    
    T& operator[](size_t index) { return mData[index]; }
    const T& operator[](size_t index) const { return mData[index]; }
    
    T& at(size_t index) { return mData[index]; }
    
    const T& at(size_t index) const { return mData[index]; }
    
    T& front() { return mData[0]; }
    const T& front() const { return mData[0]; }
    T& back() { return mData[mCurrentLength - 1]; }
    const T& back() const { return mData[mCurrentLength - 1]; }
    
    // Capacity functions
    bool empty() const { return mCurrentLength == 0; }
    size_t size() const { return mCurrentLength; }
    size_t max_size() const { return MaxLength; }
    bool full() const { return mCurrentLength == MaxLength; }
    
    // Modifiers
    void push_back(const T& value)
    {
        if (full()) return;
        
        mData[mCurrentLength++] = value;
    }
    
    void clear() { mCurrentLength = 0; }
    
    // Insert into sorted position
    void insert_sorted(const T& value, bool descending = false)
    {
        size_t i = mCurrentLength;
        
        if (i == MaxLength) // At max length we pop last element.
        {
            if ((value < mData[i-1]) != descending)
            {
                // Delete last element.
                i--;
                mCurrentLength--;
            }
            else
            {
                return;
            }
        }

        while (i > 0 && ((value < mData[i-1]) != descending))
        {
            mData[i] = mData[i-1];
            i--;
        }

        mData[i] = value;
        mCurrentLength++;
    }
};

#endif // FIXED_ARRAY_H