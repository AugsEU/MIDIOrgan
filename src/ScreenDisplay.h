#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#ifndef SCREEN_DISPLAY_H
#define SCREEN_DISPLAY_H

enum ScreenPage
{
    SP_GENERAL_INFO,
    SP_PEDAL_INFO,
};

extern ScreenPage gCurrScreenPage;
extern LiquidCrystal_I2C gLcd;

void LcdInit();
void UpdateScreen();
void SetScreenPage(ScreenPage page);

void WriteSpecialChar(uint8_t id, uint8_t x, uint8_t y);

void SetLines(const char* topLine, const char* botLine);

void EnterGeneralInfo();
void WriteGeneralInfo();

void EnterPedalInfo();
void WritePedalInfo();

#endif // SCREEN_DISPLAY_H]