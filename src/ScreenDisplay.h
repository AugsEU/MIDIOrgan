#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#ifndef SCREEN_DISPLAY_H
#define SCREEN_DISPLAY_H

extern LiquidCrystal_I2C gLcd;

// template<typename T>
// class LcdUpdateSection
// {
// private:
//     T mLastValue = 0;
//     bool mForceUpdate = true;

// public:
//     void WriteToLcd(uint8_t x, uint8_t y, T value, char* buffer, uint8_t len)
//     {
//         if (value == mLastValue && !mForceUpdate)
//         {
//             return;
//         }

//         // Store new value
//         mLastValue = value;
//         mForceUpdate = false;

//         for (uint8_t i = 0; i < len; i++)
//         {
//             buffer[i] = ' ';
//         }
//         buffer[len] = '\0';

//         // Handle negative numbers
//         char* writePtr = buffer;
//         if (value < 0)
//         {
//             *writePtr++ = '-';
//             value = -value;
//         }

//         // Convert digits
//         char* start = writePtr;
//         do
//         {
//             *writePtr++ = '0' + static_cast<char>(value % 10);
//             value /= 10;
//         } while (value > 0);

//         // Reverse digits
//         char* end = writePtr - 1;
//         while (start < end)
//         {
//             char tmp = *start;
//             *start = *end;
//             *end = tmp;
//             start++;
//             end--;
//         }

//         // Update LCD
//         gLcd.setCursor(x, y);
//         gLcd.print(buffer);
//     }

//     void ForceNextUpdate()
//     {
//         mForceUpdate = true;
//     }
// };

void LcdInit();
void UpdateScreen();
void PushDesiredChars();

void SetLines(const char* topLine, const char* botLine);

void EnterGeneralInfo();
void WriteGeneralInfo();

#endif // SCREEN_DISPLAY_H]