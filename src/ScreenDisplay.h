#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#ifndef SCREEN_DISPLAY_H
#define SCREEN_DISPLAY_H

enum ScreenPage
{
    SP_GENERAL_INFO,
    SP_PEDAL_INFO,
    SP_AUG_SYNTH_EDIT
};

extern ScreenPage gCurrScreenPage;
extern LiquidCrystal_I2C gLcd;

void LcdInit();
void UpdateScreen();
void SetScreenPage(ScreenPage page);

#endif // SCREEN_DISPLAY_H]