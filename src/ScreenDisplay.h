#include <Arduino.h>

#ifndef SCREEN_DISPLAY_H
#define SCREEN_DISPLAY_H

void LcdInit();
void WriteToLcd(const char* message);

#endif // SCREEN_DISPLAY_H