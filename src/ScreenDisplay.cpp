#include <ScreenDisplay.h>
#include <Wire.h>
#include <Globals.h>
#include <TimeInfo.h>
#include <Tempo.h>
#include <MidiOutput.h>

constexpr uTimeMs REFRESH_PERIOD = 10; // 10ms for 100FPS.
constexpr uint8_t SCREEN_WIDTH = 16;
constexpr uint8_t SCREEN_HEIGHT = 16;
constexpr uint8_t SCREEN_ADR = 0x27;

// Screen templates
//                                                          0123456789012345        
constexpr const char* const SCREEN_GENERAL_INFO_TOP_LINE = "Temp Oc    Ch   ";
constexpr const char* const SCREEN_GENERAL_INFO_BOT_LINE = "     |     |    ";

LiquidCrystal_I2C gLcd(SCREEN_ADR, SCREEN_WIDTH,SCREEN_HEIGHT); 
uTimeMs gLastUpdateTime;
char gScreenMsgBuff[SCREEN_WIDTH + 1]; // Screen is 16 long + zero terminator

uint8_t gLcdSectionWrite = 0;
LcdUpdateSection<uint16_t> gTempoSection;
LcdUpdateSection<int8_t> gUpperOctSection;
LcdUpdateSection<int8_t> gLowerOctSection;
LcdUpdateSection<uint8_t> gUpperChSection;
LcdUpdateSection<uint8_t> gLowerChSection;

void LcdInit()
{
    gLcd.init();
    gLcd.backlight();  
    gScreenMsgBuff[SCREEN_WIDTH] = '\0';

    EnterGeneralInfo();
}

void UpdateScreen()
{
    if(gTime - gLastUpdateTime < REFRESH_PERIOD)
    {
        return;
    }

    gLastUpdateTime = gTime;

    WriteGeneralInfo();
    
    return;
}

void EnterGeneralInfo()
{
    gLcd.setCursor(0, 0);
    gLcd.print(SCREEN_GENERAL_INFO_TOP_LINE);
    gLcd.setCursor(0, 1);
    gLcd.print(SCREEN_GENERAL_INFO_BOT_LINE);

    gLcdSectionWrite = 0;
}

void WriteGeneralInfo()
{
    // Screen layout
    // General Info
    //  0123456789012345
    //0 Temp Oc 16 Ch -3 
    //1 1200 | 12  | -3 

    char buffer[5];

    switch (gLcdSectionWrite)
    {
    // Top line
    case 0:
        gUpperOctSection.WriteToLcd(8, 0, gUpperOct.mValue + 1, buffer, 2);
        break;
    case 1:
        gUpperChSection.WriteToLcd(14, 0, gUpperCh.mValue, buffer, 2);
        break;
    // Bottom line.
    case 2:
        RecalculateTempo();
        gTempoSection.WriteToLcd(0, 1, gTempoCache, buffer, 4);
        break;
    case 3:
        gLowerOctSection.WriteToLcd(7, 1, gLowerOct.mValue - 3, buffer, 2);
        break;
    case 4:
        gLowerChSection.WriteToLcd(13, 1, gLowerCh.mValue, buffer, 2);
        break;
    }

    gLcdSectionWrite = (gLcdSectionWrite + 1) % 5;
}

