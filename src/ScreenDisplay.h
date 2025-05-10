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
void PushDesiredChars();
void SetScreenPage(ScreenPage page);

void SetLines(const char* topLine, const char* botLine);

void EnterGeneralInfo();
void WriteGeneralInfo();

void EnterPedalInfo();
void WritePedalInfo();

#endif // SCREEN_DISPLAY_H]