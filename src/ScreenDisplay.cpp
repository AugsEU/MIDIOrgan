#include <ScreenDisplay.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Globals.h>
#include <TimeInfo.h>
#include <Tempo.h>
#include <MidiOutput.h>

constexpr uTimeMs REFRESH_PERIOD = 100; // 100ms for 10FPS.
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

uint16_t gLastTempoCache = 0xFFFF;
int8_t gLastUpperOct = -127;
int8_t gLastLowerOct = -127;
uint8_t gLastUpperCh = 255;
uint8_t gLastLowerCh = 255;

void LcdInit()
{
    gLcd.init();
    gLcd.backlight();  
    gLcd.setCursor(0, 0);
    gLcd.print(SCREEN_GENERAL_INFO_TOP_LINE);
    gLcd.setCursor(0, 1);
    gLcd.print(SCREEN_GENERAL_INFO_BOT_LINE);

    gScreenMsgBuff[SCREEN_WIDTH] = '\0';
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

/// @brief Write a number inplace for a string buff. NOT SAFE!!
template<typename T>
void WriteNumToScreen(T num, char* buff, uint8_t len)
{
    char* writePtr = buff;
    for(uint8_t i = 0; i < len; i++)
    {
        buff[i] = ' ';
    }

    buff[len] = '\0';
    
    // Handle negative numbers
    if (num < 0)
    {
        *writePtr++ = '-';
        num = -num;
    }

    char* start = writePtr;

    // Convert digits (0 is handled specially to avoid empty buffer)
    do
    {
        *writePtr++ = '0' + static_cast<char>(num % 10);
        num /= 10;
    } while (num > 0);

    // Reverse digits
    char* end = writePtr - 1;
    while (start < end)
    {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }

    gLcd.print(buff);
}

void WriteGeneralInfo()
{
    // Screen layout
    // General Info
    //  0123456789012345
    //0 Temp Oc 16 Ch -3 
    //1 1200 | 12  | -3 

    char buffer[5];
    RecalculateTempo();

    // Top line
    gLcd.setCursor(8, 0);
    WriteNumToScreen(gUpperOct + 1, buffer, 2);

    gLcd.setCursor(14, 0);
    WriteNumToScreen(gUpperCh, buffer, 2);

    // // Bottom line.
    gLcd.setCursor(0, 1);
    WriteNumToScreen(gTempoCache, buffer, 4);

    gLcd.setCursor(7, 1);
    WriteNumToScreen(gLowerOct-3, buffer, 2);

    gLcd.setCursor(13, 1);
    WriteNumToScreen(gLowerCh, buffer, 2);
}

