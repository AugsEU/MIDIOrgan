#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#ifndef SCREEN_DISPLAY_H
#define SCREEN_DISPLAY_H

enum ScreenPage
{
    SP_PEDAL_INFO,
    SP_AUG_SYNTH_EDIT,
    SP_AUG_SYNTH_LOAD,
    SP_AUG_SYNTH_SAVE,
    SP_SEQUENCER_EDIT,
    NUM_SP_SUBSCREENS,
    SP_GENERAL_INFO = NUM_SP_SUBSCREENS
};

extern ScreenPage gCurrScreenPage;
extern LiquidCrystal_I2C gLcd;

void LcdInit();
void UpdateScreen();
void SetScreenPage(ScreenPage page);

void PlaceScreenCursor(uint8_t x, uint8_t y);
void EnableScreenCursor(bool cursor);

#endif // SCREEN_DISPLAY_H