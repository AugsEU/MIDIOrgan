#include <ScreenDisplay.h>
#include <Wire.h>
#include <Util/Globals.h>
#include <Util/TimeInfo.h>
#include <Tempo.h>
#include <MidiOutput.h>
#include <AugSynth.h>
#include <Util/LocStrings.h>
#include <Sequencer.h>

struct CharmapInfo
{
    uint8_t mLoadedSlot = 0xFF;
};

/// ===================================================================================
/// Constants
/// ===================================================================================
#define SCREEN_FLASH_TEST 0
#define CREATE_CHAR_MAP(_id_, _name_, ...) \
    static_assert(_id_<NUM_CHAR_MAPS); \
    constexpr uint8_t CID_##_name_ = _id_; \
    const uint8_t CM_##_name_[8] = { __VA_ARGS__ };

#define DEFINE_CHAR_ARRAY_ELEM(_id_, _name_, ...) \
    CM_##_name_,

#define CHAR_MAPS_TABLE(TABLE_ENTRY_FUNC) \
    TABLE_ENTRY_FUNC(0, QUARTER_NOTE, 0x2, 0x2, 0x2, 0x2, 0x6, 0xE, 0xC, 0x0)           \
    TABLE_ENTRY_FUNC(1, CHANNEL, 0x1C, 0x10, 0x1C, 0x0, 0x5, 0x7, 0x5, 0x0)             \
    TABLE_ENTRY_FUNC(2, OCTAVE, 0x1C, 0x14, 0x1C, 0x0, 0x7, 0x4, 0x7, 0x0)              \
    TABLE_ENTRY_FUNC(3, PEDAL, 0x0, 0x0, 0x1, 0x2, 0x4, 0xA, 0x12, 0x1F)                \
    TABLE_ENTRY_FUNC(4, ASP_GENERAL_U, 0x1C, 0x14, 0x1C, 0x14, 0x0, 0x10, 0x10, 0x1C)   \
    TABLE_ENTRY_FUNC(5, ASP_GENERAL_L, 0x10, 0x10, 0x1C, 0x0, 0x0, 0x1C, 0x1C, 0x0)     \
    TABLE_ENTRY_FUNC(6, ASP_DELAY_U, 0x18, 0x14, 0x14, 0x18, 0x0, 0x10, 0x10, 0x1C)     \
    TABLE_ENTRY_FUNC(7, ASP_DELAY_L, 0x14, 0x8, 0x8, 0x0, 0x10, 0x10, 0x14, 0x15)       \
    TABLE_ENTRY_FUNC(8, ASP_OSC_U, 0x1C, 0x14, 0x14, 0x1C, 0x0, 0xC, 0x8, 0x18)         \
    TABLE_ENTRY_FUNC(9, ASP_OSC1_L, 0x1C, 0x10, 0x1C, 0x0, 0x4, 0xC, 0x4, 0x4)          \
    TABLE_ENTRY_FUNC(10, ASP_OSC2_L, 0x1C, 0x10, 0x1C, 0x0, 0xC, 0x4, 0x8, 0xC)         \
    TABLE_ENTRY_FUNC(11, ASP_FILTER_U, 0x1C, 0x10, 0x18, 0x10, 0x0, 0x10, 0x10, 0x1C)   \
    TABLE_ENTRY_FUNC(12, ASP_FILTER_L, 0x1C, 0x8, 0x8, 0x0, 0x4, 0x1A, 0x1, 0x0)        \
    TABLE_ENTRY_FUNC(13, ASP_LFO_U, 0x10, 0x10, 0x10, 0x1C, 0x0, 0x1C, 0x18, 0x10)      \
    TABLE_ENTRY_FUNC(14, ASP_LFO_L, 0x1C, 0x14, 0x1C, 0x0, 0x8, 0x15, 0x2, 0x0)         \

constexpr uint8_t MAX_CHAR_STORAGE = 8;
constexpr size_t NUM_CHAR_MAPS = 15;
CHAR_MAPS_TABLE(CREATE_CHAR_MAP)
constexpr const uint8_t* CID_TO_CM[] = {CHAR_MAPS_TABLE(DEFINE_CHAR_ARRAY_ELEM)};

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
char gDesiredChars[NUM_SCREEN_CHARS]; // Screen is 32 chars
char gCurrentChars[NUM_SCREEN_CHARS]; // Screen is 32 chars
CharmapInfo gCharmapInfos[NUM_CHAR_MAPS];
uint8_t gLoadedCharmaps[MAX_CHAR_STORAGE];

ScreenPage gCurrScreenPage;
uTimeMs gPageChangeTime;

uint8_t gCursorStateCache = 0; // Setting cursor takes a long time. We store this so we only set it when necessary. low 7-bits: cursor index, high bit: cursor enabled.

/// ===================================================================================
/// Private functions
/// ===================================================================================
void UpdateFreeCharmaps();
bool PushDesiredChars();

void WriteSpecialChar(uint8_t x, uint8_t y, uint8_t id);
void SetLines(const char* topLine, const char* botLine);


void EnterGeneralInfo();
void WriteGeneralInfo();

void EnterPedalInfo();
void WritePedalInfo();

void EnterSynthEdit();
void WriteSynthParam(uint8_t x, uint8_t y, AugSynthParam* param);
void WriteSynthEdit();

void EnterSeqEdit();
void WriteSeqEdit();

/// ===================================================================================
/// Init
/// ===================================================================================

/// @brief Init LCD
void LcdInit()
{
    gLcd.init();
    gLcd.backlight();
    gLcd.blink_on();
    gLcd.cursor_off();
    gLcd.setCursor(0,0);
    for(uint8_t i = 0; i < MAX_CHAR_STORAGE; i++)
    {
        gLoadedCharmaps[i] = 0xFF;
    }

    const uint8_t NUM_SCREEN_CHARS = SCREEN_WIDTH * SCREEN_HEIGHT;
    for(uint8_t i = 0; i < NUM_SCREEN_CHARS; i++)
    {
        gDesiredChars[i] = ' ';
        gCurrentChars[i] = ' ';
    }

    UpdateFreeCharmaps();
    // Default general info
    EnterGeneralInfo();
    gPageChangeTime = gTime;
    gCurrScreenPage = ScreenPage::SP_GENERAL_INFO;
}


/// @brief Turn blinky cursor on or off
void EnableScreenCursor(bool cursor)
{
    bool cursorAlreadyOn = (gCursorStateCache & 0x80) != 0;
    if(cursor && !cursorAlreadyOn)
    {
        gLcd.cursor_on();
        gCursorStateCache |= 0x80;
    }
    else if(!cursor && cursorAlreadyOn)
    {
        gLcd.cursor_off();
        gCursorStateCache &= 0x7F;
    }
}

/// @brief Position cursor
void PlaceScreenCursor(uint8_t x, uint8_t y)
{
    uint8_t currIdx = gCursorStateCache & 0x7F;
    uint8_t newIdx = y*SCREEN_WIDTH * x;
    if(currIdx != newIdx)
    {
        gCursorStateCache = (gCursorStateCache & 0x80) | (newIdx);
        gLcd.setCursor(x, y);
    }
}

/// ===================================================================================
/// Update
/// ===================================================================================

/// @brief Change page
void SetScreenPage(ScreenPage page)
{
    ClearRotaryEncoderDeltas();
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
        case SP_AUG_SYNTH_EDIT:
            EnterSynthEdit();
            break;
        case SP_SEQUENCER_EDIT:
            EnterSeqEdit();
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
        break;
    case ScreenPage::SP_AUG_SYNTH_EDIT:
        WriteSynthEdit();
        break;
    case ScreenPage::SP_SEQUENCER_EDIT:
        WriteSeqEdit();
        break;
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
    if(PushDesiredChars())
    {

        // Update free slots
        UpdateFreeCharmaps();
    }

    
    return;
}

/// @brief Push chars in desired buffer to screen if different.
// Return true if anything changed.
bool PushDesiredChars()
{
    uint8_t numChanged = 0;

    for(uint8_t y = 0; y < SCREEN_HEIGHT; y++)
    {
        uint8_t lastWrittenIdx = 254;
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

                lastWrittenIdx = idx;
                gLcd.print(desired);
                numChanged++;

                if(numChanged >= MAX_CHANGE_PER_UPDATE)
                {
                    return true;
                }
            }
        }
    }

    return numChanged > 0;
}


/// ===================================================================================
/// Special chars
/// ===================================================================================
void UpdateFreeCharmaps()
{
    for(uint8_t i = 0; i < NUM_SCREEN_CHARS; i++)
    {
        uint8_t desired = (uint8_t)gDesiredChars[i];
        if(desired < MAX_CHAR_STORAGE)// Is a special char
        {
            gLoadedCharmaps[desired] |= 0x80;// Mark as needed.
        }

        uint8_t onScreen = (uint8_t)gCurrentChars[i];
        if(onScreen < MAX_CHAR_STORAGE)// Is a special char
        {
            gLoadedCharmaps[onScreen] |= 0x80;// Mark as needed.
        }
    }

    for(uint8_t i = 0; i < MAX_CHAR_STORAGE; i++)
    {
        if((gLoadedCharmaps[i] & 0x80) == 0)
        {
            gCharmapInfos[gLoadedCharmaps[i]].mLoadedSlot = 0xFF;
            gLoadedCharmaps[i] = 0xFF;
        }
        else
        {
            gLoadedCharmaps[i] &= 0x7F; // Unset flag bit
        }
    }
}

/// @brief Write a special character. May need to load it.
void WriteSpecialChar(uint8_t x, uint8_t y, uint8_t id)
{
    uint8_t idx = x + y * SCREEN_WIDTH;
    if(gCharmapInfos[id].mLoadedSlot == 0xFF)
    {
        for(uint8_t i = 0; i < MAX_CHAR_STORAGE; i++) // Search for free slot.
        {
            if(gLoadedCharmaps[i] == 0xFF)
            {
                gLcd.createChar(i, CID_TO_CM[id]);
                gLoadedCharmaps[i] = id;
                gCharmapInfos[id].mLoadedSlot = i;
                break;
            }
        }

        if(gCharmapInfos[id].mLoadedSlot == 0xFF)
        {
            Serial.println("ERROR: Could not find free slot for special char.");
            return;
        }
    }

    gDesiredChars[idx] = (char)(gCharmapInfos[id].mLoadedSlot);
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

/// @brief Clear an entire line
void ClearLine(uint8_t y)
{
    for(uint8_t i = 0; i < SCREEN_WIDTH; i++)
    {
        gDesiredChars[i + y * SCREEN_WIDTH] = ' ';
    }
}


/// @brief Write a number of spaces
void ClearSpace(uint8_t x, uint8_t y, uint8_t len)
{
    for(uint8_t i = 0; i < len; i++)
    {
        gDesiredChars[x + i + y * SCREEN_WIDTH] = ' ';
    }
}


template<typename T>
void WriteNumber(uint8_t x, uint8_t y, T value, uint8_t maxDigits, bool spaceNegative = false)
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
    else if(spaceNegative)
    {
        *writePtr++ = ' ';
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
void WriteString(uint8_t x, uint8_t y, const char* const buffer, uint8_t len)
{
    // Handle negative numbers
    uint8_t idx = y * SCREEN_WIDTH + x;
    const char* readPtr = buffer;
    char* writePtr = gDesiredChars + idx;
    char* writeEnd = writePtr + len;
    char* safetyEnd = gDesiredChars + NUM_SCREEN_CHARS;
    if(writeEnd > safetyEnd)
    {
        writeEnd = safetyEnd;
    }

    while (writePtr < writeEnd)
    {
        char toWrite = *readPtr;
        if(toWrite == '\0')
        {
            break;
        }

        *writePtr++ = toWrite;
        readPtr++;
    }

    while(writePtr < writeEnd)
    {
        *writePtr++ = ' ';
    }
}

/// @brief Write a single character to the screen.
void WriteChar(uint8_t x, uint8_t y, char c)
{
    gDesiredChars[y*SCREEN_WIDTH + x] = c;
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
    //0  Q  O -2 C 12 MM
    //1 999  -2   12 Pvv

    ClearLine(0);
    ClearLine(1);
    WriteSpecialChar(1, 0, CID_QUARTER_NOTE);
    WriteSpecialChar(4, 0, CID_OCTAVE);
    WriteSpecialChar(9, 0, CID_CHANNEL);
    WriteSpecialChar(13, 1, CID_PEDAL);

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
            WriteString(5, 1, "Lower+Upper", 11);
            break;
        default:
            uint8_t ch = ((uint8_t)pedalCh + 1) - PMC_MIDI_CH1;
            WriteString(7, 1, "MIDI", 4);
            WriteSpecialChar(11, 1, CID_CHANNEL);

            WriteNumber(14, 1, ch, 2);
            break;
        }
    }

    ReturnToScreenAfterTime(ScreenPage::SP_GENERAL_INFO, 3000);
}

/// ===================================================================================
/// Synth edit
/// ===================================================================================

/// @brief Called when entering the synth edit page
void EnterSynthEdit()
{

}

/// @brief Called each frame to write
void WriteSynthEdit()
{
    AugSynthPageParams& currParams = GetCurrPageParams();
    ClearLine(0);
    ClearLine(1);
    uint8_t leftWrite = 1;
    uint8_t rightWrite = 9;
    // Draw side header
    switch (currParams.mPageType)
    {
    case AugSynthPageType::ASP_GENERAL:
        WriteSpecialChar(0, 0, CID_ASP_GENERAL_U);
        WriteSpecialChar(0, 1, CID_ASP_GENERAL_L);
        break;
    case AugSynthPageType::ASP_OSC1:
        WriteSpecialChar(0, 0, CID_ASP_OSC_U);
        WriteSpecialChar(0, 1, CID_ASP_OSC1_L);
        break;
    case AugSynthPageType::ASP_OSC2:
        WriteSpecialChar(0, 0, CID_ASP_OSC_U);
        WriteSpecialChar(0, 1, CID_ASP_OSC2_L);
        break;
    case AugSynthPageType::ASP_VCF:
        WriteSpecialChar(0, 0, CID_ASP_FILTER_U);
        WriteSpecialChar(0, 1, CID_ASP_FILTER_L);
        break;
    case AugSynthPageType::ASP_LFO:
        WriteSpecialChar(0, 0, CID_ASP_LFO_U);
        WriteSpecialChar(0, 1, CID_ASP_LFO_L);
        break;
    case AugSynthPageType::ASP_LFO_OSC1:
        leftWrite = 2;
        WriteSpecialChar(0, 0, CID_ASP_LFO_U);
        WriteSpecialChar(0, 1, CID_ASP_LFO_L);
        WriteSpecialChar(1, 0, CID_ASP_OSC_U);
        WriteSpecialChar(1, 1, CID_ASP_OSC1_L);
        break;
    case AugSynthPageType::ASP_LFO_OSC2:
        leftWrite = 2;
        WriteSpecialChar(0, 0, CID_ASP_LFO_U);
        WriteSpecialChar(0, 1, CID_ASP_LFO_L);
        WriteSpecialChar(1, 0, CID_ASP_OSC_U);
        WriteSpecialChar(1, 1, CID_ASP_OSC2_L);
        break;
    case AugSynthPageType::ASP_LFO_FILT:
        leftWrite = 2;
        WriteSpecialChar(0, 0, CID_ASP_LFO_U);
        WriteSpecialChar(0, 1, CID_ASP_LFO_L);
        WriteSpecialChar(1, 0, CID_ASP_FILTER_U);
        WriteSpecialChar(1, 1, CID_ASP_FILTER_L);
        break;
    case AugSynthPageType::ASP_DELAY:
        WriteSpecialChar(0, 0, CID_ASP_DELAY_U);
        WriteSpecialChar(0, 1, CID_ASP_DELAY_L);
        break;
    default:
        break;
    }

    // Note: Order is important. We want left write first.
    WriteSynthParam(leftWrite, 0, currParams.mParameters[0]);
    WriteSynthParam(leftWrite, 1, currParams.mParameters[2]);

    WriteSynthParam(rightWrite, 0, currParams.mParameters[1]);
    WriteSynthParam(rightWrite, 1, currParams.mParameters[3]);
}

void WriteSynthParam(uint8_t x, uint8_t y, AugSynthParam* param)
{
    if(param == nullptr)
    {
        ClearSpace(x, y, 7);
        return;
    }

    switch (param->mParamNum) // Int params
    {
        case ASP_TUNING:
            WriteString(x, y, TuningToString(param->mValue), 7);
            return;
        case ASP_DELAY_MODE:
            WriteString(x, y, DelayModeToString(param->mValue), 7);
            return;
        case ASP_SOUND_TYPE:
            WriteString(x, y, SoundTypeToString(param->mValue), 7);
            return;
        case ASP_DCO_WAVE_TYPE_1:
        case ASP_DCO_WAVE_TYPE_2:
        case ASP_LFO_WAVE_TYPE:
            WriteString(x, y, OscModeToString(param->mValue), 7);
            return;
        case ASP_VCF_MODE:
            WriteString(x, y, FilterModeToString(param->mValue), 7);
            return;
        default:
            break;
    }

    uint8_t numberOffset = 4;
    const char* const paramName = AugNumberParamToString(param->mParamNum);

    switch (param->mParamNum) 
    {
        case ASP_LFO_OSC1_VOLUME:
        case ASP_LFO_OSC2_VOLUME:
            numberOffset = 3;
            break;
    }

    WriteString(x, y, paramName, 4);
    if(gpPedalInternalParam == param)
    {
        WriteString(x + numberOffset, y, "--", 2);
    }
    else
    {
        WriteNumber(x + numberOffset, y, param->mValue, 2, true);
    }
}

/// ===================================================================================
/// Sequencer edit
/// ===================================================================================
void EnterSeqEdit()
{
    PlaceScreenCursor(0, 0);
}

void WriteSeqEdit()
{
    if(IsOnTrackEditPage())
    {
        // Track edit page
        SequencerTrack* track = GetCurrSequencerTrack();
        WriteString(0, 0, "Track", 5);
        WriteNumber(6, 0, GetCurrSequencerTrackIdx()+1, 1);

        WriteString(9, 0, "Divi", 4);
        WriteNumber(14, 0, track->mSubDiv, 2);

        WriteString(0, 1, "Midi", 4);
        WriteSpecialChar(4, 1, CID_CHANNEL);
        WriteNumber(6, 1, track->mMidiCh, 2);

        WriteString(9, 1, "Len", 3);
        WriteNumber(14, 1, track->mNumSteps, 2);
    }
    else
    {
        int8_t stepIdx = GetCurrSequencerStepIdx();
        int8_t page = stepIdx >> 4; //div 16
        int8_t subPageIdx = stepIdx - (page<<4);

        // Step edit
        for(uint8_t i = 0; i < 16; i++)
        {
            bool mul4 = i % 4 == 0; //Multiples of 4 have a special comma notation
            SequencerStep* step = GetCurrSequencerStep(i + (page<<4));
            if(step->mNotes[0]==0)//empty step
            {
                WriteChar(i, 0, mul4 ? ',' : '.');
            }
            else
            {
                WriteChar(i, 0, mul4 ? ';' : ':');
            }
        }

        SequencerStep* selStep = GetCurrSequencerStep(stepIdx);

        WriteChar(0, 1, '0'+(char)page);
        WriteString(1, 1, ") Vel", 6);
        WriteNumber(7, 1, selStep->mVelocity, 2);
        WriteString(10, 1, "Len", 4);
        WriteNumber(14, 1, selStep->mLength, 2);

        PlaceScreenCursor(subPageIdx, 0);
    }
}