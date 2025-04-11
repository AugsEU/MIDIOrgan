#include <Arduino.h>

template<typename T>
inline void dbgPrint(const char* const msg, T data)
{
    Serial.print(msg);
    Serial.print(": ");
    Serial.print(data);
    Serial.print("\n");
}