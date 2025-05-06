#include <ScreenDisplay.h>
#include <Wire.h>
#include <Globals.h>
#include <TimeInfo.h>
#include <Tempo.h>
#include <MidiOutput.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
#define SCREEN_FLASH_TEST 0

const char CM_QUARTER_NOTE[8] PROGMEM = { 0x2, 0x2, 0x2, 0x2, 0x6, 0xE, 0xC, 0x0 };
const char CM_CHANNEL[8] PROGMEM = { 0x1C, 0x10, 0x1C, 0x0, 0x5, 0x7, 0x5, 0x0 };
const char CM_OCTAVE[8] PROGMEM = { 0x1C, 0x14, 0x1C, 0x0, 0x7, 0x4, 0x7, 0x0 };
//const char CM_PEDAL[8] PROGMEM = { 0x1C, 0x14, 0x19, 0x12, 0x4, 0xA, 0x12, 0x1F }; // with P
const char CM_PEDAL[8] PROGMEM = { 0x0, 0x0, 0x1, 0x2, 0x4, 0xA, 0x12, 0x1F }; // without P

constexpr uTimeMs REFRESH_PERIOD = 10; // 10ms for 100FPS.
constexpr uint8_t SCREEN_WIDTH = 16;
constexpr uint8_t SCREEN_HEIGHT = 2;
constexpr uint8_t SCREEN_ADR = 0x27;
constexpr uint8_t NUM_SCREEN_CHARS = SCREEN_WIDTH * SCREEN_HEIGHT;
constexpr uint8_t MAX_CHANGE_PER_UPDATE = 3;

// Screen templates
//  0123456789012345
//0  Q  O -2 C 12 P
//1 999  -2   12 mmm
//                                                          01   234   56789   01234   5        
constexpr const char* const SCREEN_GENERAL_INFO_TOP_LINE = " \x01  \x03    \x02    \x04 ";
constexpr const char* const SCREEN_GENERAL_INFO_BOT_LINE = "                 ";

#if SCREEN_FLASH_TEST
//                                               0123456789012345
constexpr const char* const SCREEN_TEST1_LINE = "AAAAAAAAAAAAAAAA";
constexpr const char* const SCREEN_TEST2_LINE = "BBBBBBBBBBBBBBBB";
uint8_t gTestCountDown = 0;
#endif // SCREEN_FLASH_TEST

/// ===================================================================================
/// Globals
/// ===================================================================================
LiquidCrystal_I2C gLcd(SCREEN_ADR, SCREEN_WIDTH, SCREEN_HEIGHT); 
char gDesiredChars[NUM_SCREEN_CHARS + 1]; // Screen is 32 chars + zero terminator
char gCurrentChars[NUM_SCREEN_CHARS + 1]; // Screen is 32 chars + zero terminator

/// ===================================================================================
/// Init
/// ===================================================================================

/// @brief Init LCD
void LcdInit()
{
    gLcd.init();
    gLcd.backlight();
    gLcd.createChar(1, CM_QUARTER_NOTE);
    gLcd.createChar(2, CM_CHANNEL);
    gLcd.createChar(3, CM_OCTAVE);
    gLcd.createChar(4, CM_PEDAL);

    const uint8_t NUM_SCREEN_CHARS = SCREEN_WIDTH * SCREEN_HEIGHT;
    for(uint8_t i = 0; i < NUM_SCREEN_CHARS; i++)
    {
        gDesiredChars[i] = ' ';
        gCurrentChars[i] = ' ';
    }
    gDesiredChars[SCREEN_WIDTH*SCREEN_HEIGHT] = '\0';
    gCurrentChars[SCREEN_WIDTH*SCREEN_HEIGHT] = '\0';

    EnterGeneralInfo();
}

/// @brief Update screen. Must be called regularly
void UpdateScreen()
{
#if !SCREEN_FLASH_TEST
    WriteGeneralInfo();
#else 
    if(gTestCountDown == 0)
    {
        gTestCountDown = 255;
       
    }
    else
    {
        gTestCountDown--;
    }

    if(gTestCountDown < 127)
    {
        SetLines(SCREEN_TEST1_LINE, SCREEN_TEST1_LINE);
    }
    else
    {
        SetLines(SCREEN_TEST2_LINE, SCREEN_TEST2_LINE);
    }
#endif // !SCREEN_FLASH_TEST

    // Push desired chars to the screen.
    PushDesiredChars();
    
    return;
}

/// @brief Push chars in desired buffer to screen if different.
void PushDesiredChars()
{
    uint8_t numChanged = 0;

    for(uint8_t y = 0; y < SCREEN_HEIGHT; y++)
    {
        uint8_t lastWrittenIdx = 255;
        for(uint8_t x = 0; x < SCREEN_WIDTH; x++)
        {
            uint8_t idx = y * SCREEN_WIDTH + x;
            char desired = gDesiredChars[idx];
            if(desired != gCurrentChars[idx])
            {
                gCurrentChars[idx] = desired;

                if(lastWrittenIdx + 1 != idx)
                {
                    gLcd.setCursor(x, y);
                }
                gLcd.print(desired);
                numChanged++;

                if(numChanged >= MAX_CHANGE_PER_UPDATE)
                {
                    return;
                }
            }
        }
    }
}


/// ===================================================================================
/// Write to buffer
/// ===================================================================================

/// @brief Write background to buffer
void SetLines(const char* topLine, const char* botLine)
{
    for(uint8_t i = 0; i < SCREEN_WIDTH; i++)
    {
        gDesiredChars[i] = topLine[i];
    }

    for(uint8_t i = 0; i < SCREEN_WIDTH; i++)
    {
        gDesiredChars[i+SCREEN_WIDTH] = botLine[i];
    }
}

template<typename T>
void WriteNumber(uint8_t x, uint8_t y, T value, uint8_t maxDigits)
{
    // Handle negative numbers
    uint8_t idx = y * SCREEN_WIDTH + x;
    char* buffer = gDesiredChars + idx;
    const char* spaceToFill = buffer + maxDigits; 

    char* writePtr = buffer;
    if (value < 0)
    {
        *writePtr++ = '-';
        value = -value;
    }

    // Convert digits
    char* start = writePtr;
    do
    {
        *writePtr++ = '0' + static_cast<char>(value % 10);
        maxDigits--;
        value /= 10;
    } while (value > 0);

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

    while (writePtr < spaceToFill)
    {
        *writePtr++ = ' ';
    }
}


/// ===================================================================================
/// General info
/// ===================================================================================

/// @brief Enter "General info section"
void EnterGeneralInfo()
{
    SetLines(SCREEN_GENERAL_INFO_TOP_LINE, SCREEN_GENERAL_INFO_BOT_LINE);
}

/// @brief Write out the general info section
void WriteGeneralInfo()
{
    // Screen layout
    // General Info
    //  0123456789012345
    //0  Q  O -2 C 12 P
    //1 999  -2   12 vmm

    WriteNumber(6, 0, gUpperOct.mValue + 1, 2);
    WriteNumber(11, 0, gUpperCh.mValue, 2);

    RecalculateTempo();
    WriteNumber(0, 1, gTempoCache, 3);
    WriteNumber(5, 1, gLowerOct.mValue - 3, 2);

#if !AUG_SYNTH_DEBUG
    WriteNumber(10, 1, gLowerCh.mValue, 2);
#else // !AUG_SYNTH_DEBUG
    WriteNumber(13, 1, gCatSelector.mValue, 2);
#endif // !AUG_SYNTH_DEBUG

    uint32_t pedal99 = GetPedalStable();
    pedal99 *= 99;
    pedal99 /= ANALOG_MAX_VALUE;
    pedal99 = min(99, pedal99);

    WriteNumber(14, 1, pedal99, 2);
}

