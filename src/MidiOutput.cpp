#include <MidiOutput.h>
#include <MIDI.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>
#include <NotePressInfo.h>
#include <StableAnalog.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr uint8_t METRONOME_NOTE = 82;
constexpr int8_t LOWER_KEY_SHIFT = 5;
constexpr int8_t UPPER_KEY_SHIFT = 9;

constexpr uint8_t BP_CMD_NOTE_ON = 0xC0;
constexpr uint8_t BP_CMD_NOTE_OFF = 0x80;


/// ===================================================================================
/// Globals
/// ===================================================================================
NotePressInfo gNoteStates[NUM_NOTES];

AnalogSelector<uint8_t, 17, 0> gUpperCh;
AnalogSelector<uint8_t, 17, 0> gLowerCh;
AnalogSelector<int8_t, 5, -3> gUpperOct;
AnalogSelector<int8_t, 5, 1> gLowerOct;
uint8_t gPlayVelocity = 100;
uint8_t gPlayingMetronomeNote = 0;
uint8_t gBpMsgBuff[5];
MIDI_CREATE_DEFAULT_INSTANCE();

void SendMessageToBp();
void SendMessageToBp(const uint8_t paramNum, const float value);

/// ===================================================================================
/// Setup & Update
/// ===================================================================================

/// @brief Setup midi
void MidiOutputSetup()
{
    // TX1 line is the private line to the internal synth
    Serial1.begin(115200);

	MIDI.begin(MIDI_CHANNEL_OFF);

    gUpperCh.ForceSelection(gapMidiChUpper);
    gLowerCh.ForceSelection(gapMidiChLower);

    gUpperOct.ForceSelection(gapOctaveUpper);
    gLowerOct.ForceSelection(gapOctaveLower);
}



/// @brief Update midi, should be called every loop.
void UpdateMidiOutput()
{
    PlayMetronome();

    uint8_t unextValue = gUpperCh.CalcNextSelection(gapMidiChUpper);
    if(gUpperCh.mValue != unextValue)
    {
        CancelAllNotes(true);
        gUpperCh.mValue = unextValue;    
    }

    unextValue = gLowerCh.CalcNextSelection(gapMidiChLower);
    if (gLowerCh.mValue != unextValue)
    {
        CancelAllNotes(false);
        gLowerCh.mValue = unextValue;
    }

    int8_t nextValue = gUpperOct.CalcNextSelection(gapOctaveUpper);
    if (gUpperOct.mValue != nextValue)
    {
        CancelAllNotes(true);
        gUpperOct.mValue = nextValue;
    }

    nextValue = gLowerOct.CalcNextSelection(gapOctaveLower);
    if (gLowerOct.mValue != nextValue)
    {
        CancelAllNotes(false);
        gLowerOct.mValue = nextValue;
    }

    // Tempo
    if (On4Note(48))
    {
        MIDI.sendClock();
    }
}


/// @brief Play keys like a regular piano.
void PlayNotesDirect(uint8_t keyStart, uint8_t keyEnd)
{
	for (uint8_t keyNum = keyStart; keyNum < keyEnd; keyNum++)
	{
		uint8_t vPinIdx = KeyNumToVirtualPin(keyNum);
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		
        NotePressInfo* pPressInfo = &gNoteStates[vPinIdx - NOTES_VPIN_START];
        
		bool prevPressed = pPressInfo->mPressed;
		pPressInfo->ChangeState(vPinState, gTime);
		bool pressed = pPressInfo->mPressed;

		if (pressed)
		{
			if (!prevPressed)
			{
				SendNoteOn(keyNum);
			}
		}
		else
		{
			if (prevPressed)
			{
				SendNoteOff(keyNum);
			}
		}
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
    return upper ? gUpperCh.mValue : gLowerCh.mValue;
}



/// @brief Convert key number to note number
uint8_t KeyNumToNote(uint8_t keyNum)
{
    if (IsUpperKey(keyNum))
    {
        keyNum = keyNum + gUpperOct.mValue * 12 + UPPER_KEY_SHIFT;
    }
    else
    {
        keyNum = keyNum + gLowerOct.mValue * 12 + LOWER_KEY_SHIFT;
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
    uint8_t ch = GetChannel(isUpper);

    SendNoteOn(keyNum, ch);
}



/// @brief Send a keynumber
void SendNoteOff(uint8_t keyNum)
{
    bool isUpper = IsUpperKey(keyNum);
    uint8_t ch = GetChannel(isUpper);

    SendNoteOff(keyNum, ch);
}



/// @brief Send a note on.
void SendNoteOn(uint8_t keyNum, uint8_t ch)
{
    uint8_t noteNum = KeyNumToNote(keyNum);

    if (ch == 0)
    {
        gBpMsgBuff[0] = BP_CMD_NOTE_ON;
        gBpMsgBuff[1] = noteNum;
        gBpMsgBuff[2] = gPlayVelocity;
        SendMessageToBp();
    }
    else
    {
        MIDI.sendNoteOn(noteNum, gPlayVelocity, ch);
    }
}



/// @brief Send a note off.
void SendNoteOff(uint8_t keyNum, uint8_t ch)
{
    uint8_t noteNum = KeyNumToNote(keyNum);

    if (ch == 0)
    {
        gBpMsgBuff[0] = BP_CMD_NOTE_OFF;
        gBpMsgBuff[1] = noteNum;
        gBpMsgBuff[2] = gPlayVelocity;
        SendMessageToBp();
    }
    else
    {
        MIDI.sendNoteOff(noteNum, gPlayVelocity, ch);
    }
}



/// @brief Play note on every channel
void SendNoteOnAllCh(uint8_t noteNum)
{
    for(uint8_t ch = 0; ch <= 16; ch++)
    {
        SendNoteOn(noteNum, ch);
    }
}



/// @brief Cancel note on every channel
void SendNoteOffAllCh(uint8_t noteNum)
{
    for(uint8_t ch = 0; ch <= 16; ch++)
    {
        SendNoteOff(noteNum, ch);
    }
}



/// @brief Cancel notes on upper/lower keybed.
void CancelAllNotes(bool upper)
{
    static_assert(NUM_LOWER_KEYS == NUM_UPPER_KEYS && NUM_LOWER_KEYS == NUM_NOTES/2, "Fix this if constants change.");

    uint8_t keyNum = upper ? NUM_LOWER_KEYS : 0;
    for(uint8_t i = 0; i < NUM_NOTES/2; i++)
    {
        SendNoteOff(keyNum++);
    }
}



/// @brief Send a message to the BP synth
void SendMessageToBp()
{
    constexpr uint8_t BUFF_SIZE = sizeof(gBpMsgBuff);

    for (uint8_t i = 0; i < BUFF_SIZE; i++)
    {
        Serial1.write(gBpMsgBuff[i]);
    }
}


/// @brief Send a message to the BP synth
void SendMessageToBp(const uint8_t paramNum, const float value)
{
    gBpMsgBuff[0] = paramNum & 0x7F;

    float* floatPtr = reinterpret_cast<float*>(gBpMsgBuff + 1);
    *floatPtr = value;

    SendMessageToBp();
}

void SendMessageToBp(const uint8_t header)
{
    gBpMsgBuff[0] = header;
    SendMessageToBp();
}