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

// Empty
constexpr const char* const SCREEN_EMPTY_LINE = "                 ";

// General info
//  0123456789012345
//0  Q  O -2 C 12 P
//1 999  -2   12 mmm
//                                                          01   234   56789   012345        
constexpr const char* const SCREEN_GENERAL_INFO_TOP_LINE = " \x01  \x03    \x02      ";

//                                                          01234567890123   45  
constexpr const char* const SCREEN_GENERAL_INFO_BOT_LINE = "             \x04  ";


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

ScreenPage gCurrScreenPage;
uTimeMs gPageChangeTime;

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

    // Default general info
    EnterGeneralInfo();
    gPageChangeTime = gTime;
    gCurrScreenPage = ScreenPage::SP_GENERAL_INFO;
}


/// @brief Change page
void SetScreenPage(ScreenPage page)
{
    gPageChangeTime = gTime;
    if(page != gCurrScreenPage)
    {
        switch (page)
        {
        case SP_GENERAL_INFO:
            EnterGeneralInfo();
            break;
        case SP_PEDAL_INFO:
            EnterPedalInfo();
            break;
        }
    }
    gCurrScreenPage = page;
}


/// @brief Return to another screen after a time has expired
void ReturnToScreenAfterTime(ScreenPage page, uTimeMs timeMs)
{
    if(gTime - gPageChangeTime > timeMs)
    {
        SetScreenPage(page);
    }
}


/// @brief Update screen. Must be called regularly
void UpdateScreen()
{
#if !SCREEN_FLASH_TEST
    switch (gCurrScreenPage)
    {
    case ScreenPage::SP_GENERAL_INFO:
        WriteGeneralInfo();
        break;
    case ScreenPage::SP_PEDAL_INFO:
        WritePedalInfo();
    default:
        break;
    }
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

void ClearLine(uint8_t y)
{
    for(uint8_t i = 0; i < SCREEN_WIDTH; i++)
    {
        gDesiredChars[i + y * SCREEN_WIDTH] = ' ';
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

/// @brief Write a string at a position with whitespace
void WriteString(uint8_t x, uint8_t y, const char* buffer, uint8_t len)
{
    // Handle negative numbers
    uint8_t idx = y * SCREEN_WIDTH + x;
    char* writePtr = gDesiredChars + idx;
    char* writeEnd = writePtr + len;
    char* safetyEnd = gDesiredChars + NUM_SCREEN_CHARS;

    while (len > 0 && writePtr < writeEnd)
    {
        char toWrite = *buffer;
        if(toWrite == '\0')
        {
            break;
        }

        *writePtr++ = toWrite;
        buffer++;

        if(writePtr > safetyEnd)
        {
            return;
        }
    }

    while(writePtr < writeEnd)
    {
        *writePtr++ = ' ';
    }
}


/// ===================================================================================
/// General info
/// ===================================================================================

const char* const PedalModeToCharsShort(PedalMode mode);

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
    WriteString(14, 0, PedalModeToCharsShort(GetPedalMode()), 2);
}

/// ===================================================================================
/// Pedal Info
/// ===================================================================================

/// @brief Enter "General info section"
void EnterPedalInfo()
{
    SetLines(SCREEN_EMPTY_LINE, SCREEN_EMPTY_LINE);
}

/// @brief Convert pedal mode to string (max 10 len)
const char* const PedalModeToChars(PedalMode mode)
{
    switch (mode)
    {    
        case PM_OFF:
            return "Off";
        case PM_VELOCITY:
            return "Velocity";
        case PM_PITCH_BEND:
            return "Pitch Bend";
        case PM_MODULATION:
            return "Modulation";
        case PM_VOLUME:
            return "Volume";
        case PM_INTERNAL:
            return "Internal";
        default:
            break;
    }

    return "!ERROR!";
}

/// @brief Convert pedal mode to string (max 2 len)
const char* const PedalModeToCharsShort(PedalMode mode)
{
    switch (mode)
    {    
        case PM_OFF:
            return "--";
        case PM_VELOCITY:
            return "Ve";
        case PM_PITCH_BEND:
            return "Pb";
        case PM_MODULATION:
            return "Md";
        case PM_VOLUME:
            return "Vo";
        case PM_INTERNAL:
            return "In";
        default:
            break;
    }

    return "!ERROR!";
}

/// @brief Convert pedal internal param to string
const char* const PedalInternalToString(PedalInternalParam param)
{
    switch (param)
    {    
        case PIP_GAIN:
            return "Gain";
        case PIP_DELAY_TIME:
            return "Delay Time";
        case PIP_DELAY_FEEDBACK:
            return "Delay Feedback";
        case PIP_DCO_VOL_1:
            return "Osc1 Volume";
        case PIP_DCO_VOL_2:
            return "Osc2 Volume";
        case PIP_CUTOFF:
            return "Filter Freq";
        case PIP_LFO_RATE:
            return "LFO Speed";
        case PIP_LFO_OSC1_FREQ:
            return "LFO Osc1 Freq";
        case PIP_LFO_OSC2_FREQ:
            return "LFO Osc2 Freq";
        default:
            break;
    }

    return "!ERROR!";
}

/// @brief Write out the general info section
void WritePedalInfo()
{
    PedalMode mode = GetPedalMode();
    WriteString(0, 0, PedalModeToChars(mode), 10);

    ClearLine(1);
    if (mode == PedalMode::PM_INTERNAL)
    {
        const char* paramStr = PedalInternalToString(GetPedalInternalParam());
        uint8_t paramStrLen = strlen(paramStr);
        WriteString(16-paramStrLen, 1, paramStr, paramStrLen);
    }
    else
    {
        //WriteString(11, 0, "MIDI\x02", 5);
        PedalMidiCh pedalCh = GetPedalMidiCh();

        switch (pedalCh)
        {
        case PMC_LOWER:
            WriteString(11, 1, "Lower", 5);
            break;
        case PMC_UPPER:
            WriteString(11, 1, "Upper", 5);
            break;
        case PMC_UPPER_LOWER:
            WriteString(5, 1, "Upper+Lower", 11);
            break;
        default:
            uint8_t ch = ((uint8_t)pedalCh + 1) - PMC_MIDI_CH1;
            WriteString(7, 1, "MIDI \x02", 6);
            WriteNumber(14, 1, ch, 2);
            break;
        }
    }

    ReturnToScreenAfterTime(ScreenPage::SP_GENERAL_INFO, 2000);
}