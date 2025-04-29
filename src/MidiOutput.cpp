#include <MidiOutput.h>
#include <MIDI.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>
#include <UserControls.h>
#include <StableAnalog.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr uint8_t METRONOME_NOTE = 82;
constexpr int8_t LOWER_KEY_SHIFT = 5;
constexpr int8_t UPPER_KEY_SHIFT = 9;



/// ===================================================================================
/// Globals
/// ===================================================================================
uint8_t gUpperCh = 1;
uint8_t gLowerCh = 1;
int8_t gUpperOct = -1;
int8_t gLowerOct = 3;
uint8_t gPlayVelocity = 100;
uint8_t gPlayingMetronomeNote = 0;
MIDI_CREATE_DEFAULT_INSTANCE();



/// ===================================================================================
/// Setup & Update
/// ===================================================================================

/// @brief Setup midi
void MidiOutputSetup()
{
	MIDI.begin(MIDI_CHANNEL_OFF);

    gUpperOct = GetAnalogSelectionValue(gapOctaveUpper, 5) - 3;
    gLowerOct = GetAnalogSelectionValue(gapOctaveLower, 5) + 1;
}



/// @brief Update midi, should be called every loop.
void UpdateMidiOutput()
{
    PlayMetronome();

    // Upper: -3, -2, -1, 0, 1
    // Lower: 1, 2, 3, 4, 5
    int8_t newUpOct = 0, newLowOct = 0;
    
    // Cast is ok because values are small
    if (UpdateAnalogSelectionValue((uint8_t*)&newUpOct, gapOctaveUpper, 5))
    {
        newUpOct -= 3;

        if (gUpperOct != newUpOct)
        {
            CancelAllNotes(true);
        }
        gUpperOct = newUpOct;
    }
    if (UpdateAnalogSelectionValue((uint8_t*)&newLowOct, gapOctaveLower, 5))
    {
        newLowOct += 1;

        if (gLowerOct != newLowOct)
        {
            CancelAllNotes(false);
        }
        gLowerOct = newLowOct;
    }
}



/// @brief Play the metronome
void PlayMetronome()
{
    bool noteOn = On4Note(2);
    bool noteOff = !noteOn && On4Note(16);
    uint8_t note = METRONOME_NOTE;

    if (noteOff && gPlayingMetronomeNote > 0)
    {
        SendNoteOffAllCh(note);
        gPlayingMetronomeNote = 0;
    }

    if (gdpMetronome.IsActive() && noteOn)
    {
        SendNoteOnAllCh(note);
        gPlayingMetronomeNote = note;
    }
}

/// ===================================================================================
/// Utils
/// ===================================================================================

/// @brief Get the channel of either the upper or lower keybed
uint8_t GetChannel(bool upper)
{
    return upper ? gUpperCh : gLowerCh;
}



/// @brief Convert key number to note number
uint8_t KeyNumToNote(uint8_t keyNum)
{
    if (IsUpperKey(keyNum))
    {
        keyNum = keyNum + gUpperOct * 12 + UPPER_KEY_SHIFT;
    }
    else
    {
        keyNum = keyNum + gLowerOct * 12 + LOWER_KEY_SHIFT;
    }

    return keyNum;
}

/// ===================================================================================
/// Commands
/// ===================================================================================

/// @brief Send a key number
void SendNoteOn(uint8_t keyNum)
{
    bool isUpper = IsUpperKey(keyNum);
    uint8_t noteNum = KeyNumToNote(keyNum);
    MIDI.sendNoteOn(noteNum, gPlayVelocity, GetChannel(isUpper));
}



/// @brief Send a keynumber
void SendNoteOff(uint8_t keyNum)
{
    bool isUpper = IsUpperKey(keyNum);
    uint8_t noteNum = KeyNumToNote(keyNum);
    MIDI.sendNoteOff(noteNum, gPlayVelocity, GetChannel(isUpper));
}



/// @brief Play note on certain channel
void SendNoteOn(uint8_t noteNum, bool upper)
{
    MIDI.sendNoteOn(noteNum, gPlayVelocity, GetChannel(upper));
}



/// @brief Unplay note on certain channel
void SendNoteOff(uint8_t noteNum, bool upper)
{
    MIDI.sendNoteOff(noteNum, 0, GetChannel(upper));
}



/// @brief Play note on every channel
void SendNoteOnAllCh(uint8_t noteNum)
{
    for(uint8_t ch = 1; ch <= 16; ch++)
    {
        MIDI.sendNoteOn(noteNum, gPlayVelocity, ch);
    }
}



/// @brief Cancel note on every channel
void SendNoteOffAllCh(uint8_t noteNum)
{
    for(uint8_t ch = 1; ch <= 16; ch++)
    {
        MIDI.sendNoteOff(noteNum, 0, ch);
    }
}



/// @brief Cancel notes on upper/lower keybed.
void CancelAllNotes(bool upper)
{
    static_assert(NUM_LOWER_KEYS == NUM_UPPER_KEYS && NUM_LOWER_KEYS == NUM_NOTES/2, "Fix this if constants change.");

    uint8_t keyNum = upper ? NUM_LOWER_KEYS : 0;
    for(uint8_t i = 0; i < NUM_NOTES/2; i++)
    {
        uint8_t noteNum = KeyNumToNote(keyNum++);
        MIDI.sendNoteOff(noteNum, 0, GetChannel(upper));
    }
}