#include <ScreenDisplay.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Globals.h>
#include <TimeInfo.h>
#include <Tempo.h>
#include <MidiOutput.h>

constexpr uTimeMs REFRESH_PERIOD = 100; // 16ms for 60FPS.
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

void LcdInit()
{
    gLcd.init();
    gLcd.backlight();  
    gLcd.setCursor(0, 0);
    gLcd.print("Loading...");

    gScreenMsgBuff[SCREEN_WIDTH] = '\0';
}

void UpdateScreen()
{
    if(gTime - gLastUpdateTime < REFRESH_PERIOD)
    {
        return;
    }

    gLastUpdateTime = gTime;

    // Screen layout
    // General Info
    //  0123456789012345
    //0 Temp Ch 16 Oc -3 
    //1 1200 | 12  | -3 
    WriteGeneralInfo();
    
    return;
}

/// @brief Write a number inplace for a string buff. NOT SAFE!!
void WriteNumToBuffi8(int8_t num, char* buff)
{
    if (num < 0)
    {
        *buff++ = '-';
        num = -num;
    }

    char* start = buff;

    do
    {
        *buff++ = '0' + (char)(num % 10);
        num /= 10;
    }
    while (num > 0);

    char* end = buff - 1;
    while (start < end)
    {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
}

void WriteNumToBuffu16(uint16_t num, char* buff)
{
    char* start = buff;

    do
    {
        *buff = '0' + (char)(num % 10);
        buff++;
        num /= 10;
    }
    while (num > 0);

    char* end = buff - 1;
    while (start < end)
    {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
}

void WriteGeneralInfo()
{
    RecalculateTempo();

    // Top line
    strncpy(gScreenMsgBuff, SCREEN_GENERAL_INFO_TOP_LINE, SCREEN_WIDTH+1);
    WriteNumToBuffi8(gUpperOct + 1, gScreenMsgBuff + 8);
    WriteNumToBuffi8(gUpperCh + 1, gScreenMsgBuff + 14);
    gLcd.setCursor(0, 0);
    gLcd.print(gScreenMsgBuff);

    // Bottom line.
    strncpy(gScreenMsgBuff, SCREEN_GENERAL_INFO_BOT_LINE, SCREEN_WIDTH+1);
    WriteNumToBuffu16(gTempoCache, gScreenMsgBuff);
    WriteNumToBuffi8(gLowerOct, gScreenMsgBuff + 7);
    WriteNumToBuffi8(gLowerCh, gScreenMsgBuff + 13);
    gLcd.setCursor(0, 1);
    gLcd.print(gScreenMsgBuff);
}