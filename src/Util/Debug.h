#include <Arduino.h>

#define DEBUG_ASSERT 1

#ifndef DEBUG_H
#define DEBUG_H

template<typename T>
inline void DebugPrint(const char* const msg, T data)
{
    Serial.print(msg);
    Serial.print(": ");
    Serial.print(data);
    Serial.print("\n");
}

#if DEBUG_ASSERT
    #define AG_ASSERT(cond, num) if(!(cond)) DebugPrint("AST:", num)
#else 
    #define AG_ASSERT(cond, msg) do {} while(false)
#endif 

#endif // DEBUG_H