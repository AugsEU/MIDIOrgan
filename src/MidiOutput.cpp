#include <MidiOutput.h>
#include <MIDI.h>
#include <Util/VirtualPinToNote.h>
#include <Tempo.h>
#include <Input/NotePressInfo.h>
#include <Input/StableAnalog.h>
#include <AugSynthParams.h>
#include <AugSynth.h>
#include <ScreenDisplay.h>
#include <Looper.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr uint8_t METRONOME_NOTE = 82;
constexpr int8_t LOWER_KEY_SHIFT = 5;
constexpr int8_t UPPER_KEY_SHIFT = 9;
constexpr uint8_t DEFAULT_PLAY_VELOCITY = 100;

constexpr uint8_t BP_CMD_HEADER = 0xFF;
constexpr uint8_t BP_CMD_NOTE_ON = 0xC0;
constexpr uint8_t BP_CMD_NOTE_OFF = 0x80;
constexpr uint8_t BP_CMD_CLICK_ON = 0xA0;
constexpr uint8_t BP_CMD_CLICK_OFF = 0x90;


/// ===================================================================================
/// Globals
/// ===================================================================================
NotePressInfo gNoteStates[NUM_NOTES];
AnalogSelector<uint8_t, 17, 0> gUpperCh;
AnalogSelector<uint8_t, 17, 0> gLowerCh;
AnalogSelector<int8_t, 5, -3> gUpperOct;
AnalogSelector<int8_t, 5, 1> gLowerOct;

AnalogSelector<uint8_t, NUM_PEDAL_MODES, 0> gPedalModeSelect;
AnalogSelector<uint8_t, NUM_PEDAL_MIDI_CH, 0> gPedalMidiChSelect;
AnalogSelector<uint8_t, NUM_PEDAL_INTERNAL_PARAMS, 0> gPedalInternalParamSelect;

uint8_t gPedalVelocity = DEFAULT_PLAY_VELOCITY;
uint8_t gPlayingMetronomeNote = 0;
uint8_t gBpMsgBuff[5];

PedalMode gPedalMode;
PedalMidiCh gPedalMidiCh;
PedalInternalParam gPedalInternal;
AugSynthParam* gpPedalInternalParam;

#if AUG_SYNTH_DEBUG
AugSynthValueCategory gDebugSynthCat = AugSynthValueCategory::ENV;

AnalogSelector<uint8_t, 5, 0> gCatSelector;

StableAnalog gKnob2;
StableAnalog gKnob3;
StableAnalog gKnob4;
StableAnalog gKnob5;
StableAnalog gKnob6;
StableAnalog gKnob7;

uint8_t gParamToSend = 0;
void SetAugSynthParam(const AugSynthPage category, const uint8_t index, const float value);
#endif // AUG_SYNTH DEBUG

MIDI_CREATE_DEFAULT_INSTANCE();

void SendMessageToBp();
void ResetBpSynth();
void SendParameterToBp(const uint8_t paramNum, const float value);

void ResetPedalForModeChange();

/// ===================================================================================
/// Setup & Update
/// ===================================================================================

/// @brief Setup midi
void MidiOutputSetup()
{
    // TX1 line is the private line to the internal synth
    Serial1.begin(115200);

	MIDI.begin(MIDI_CHANNEL_OFF);

#if !AUG_SYNTH_DEBUG
    gUpperCh.ForceSelection(gapMidiChUpper);
    gLowerCh.ForceSelection(gapMidiChLower);

    gUpperOct.ForceSelection(gapOctaveUpper);
    gLowerOct.ForceSelection(gapOctaveLower);

    gPedalModeSelect.ForceSelection(gapPedalMode);
    gPedalMidiChSelect.ForceSelection(gapPedalSelect);
    gPedalInternalParamSelect.ForceSelection(gapPedalSelect);
#else // !AUG_SYNTH_DEBUG
    gUpperCh.ForceSelection(0);
    gLowerCh.ForceSelection(0);

    gUpperOct.ForceSelection(512);
    gLowerOct.ForceSelection(256);

    gCatSelector.ForceSelection(gapMidiChLower);
    gKnob2.ConsumeInput(gapMidiChUpper);
    gKnob3.ConsumeInput(gapOctaveLower);
    gKnob4.ConsumeInput(gapOctaveLower);
    gKnob5.ConsumeInput(gapTempo);
    gKnob6.ConsumeInput(gapKnob6);
    gKnob7.ConsumeInput(gapKnob7);
#endif // !AUG_SYNTH_DEBUG

    gPedalMode = (PedalMode)gPedalModeSelect.mValue;
    gPedalMidiCh = (PedalMidiCh)gPedalMidiChSelect.mValue;
    gPedalInternal = (PedalInternalParam)gPedalInternalParamSelect.mValue;
}



/// @brief Update midi, should be called every loop.
void UpdateMidiOutput()
{
    PlayMetronome();

#if !AUG_SYNTH_DEBUG
    // Midi ch
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

    // Pedal
    unextValue = gPedalModeSelect.CalcNextSelection(gapPedalMode);
    if (gPedalModeSelect.mValue != unextValue)
    {
        gPedalModeSelect.mValue = unextValue;
        SetScreenPage(ScreenPage::SP_PEDAL_INFO);
    }

    if (GetPedalMode() == PedalMode::PM_INTERNAL)
    {
        unextValue = gPedalInternalParamSelect.CalcNextSelection(gapPedalSelect);
        if (gPedalInternalParamSelect.mValue != unextValue)
        {
            gPedalInternalParamSelect.mValue = unextValue;
            SetScreenPage(ScreenPage::SP_PEDAL_INFO);
        }
    }
    else // Midi channel based param
    {
        unextValue = gPedalMidiChSelect.CalcNextSelection(gapPedalSelect);
        if (gPedalMidiChSelect.mValue != unextValue)
        {
            gPedalMidiChSelect.mValue = unextValue;
            SetScreenPage(ScreenPage::SP_PEDAL_INFO);
        }
    }

    // Octave
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
#else // !AUG_SYNTH_DEBUG
    gCatSelector.mValue = gCatSelector.CalcNextSelection(gapMidiChLower);

    gKnob2.ConsumeInput(gapMidiChUpper);
    gKnob3.ConsumeInput(gapOctaveLower);
    gKnob4.ConsumeInput(gapOctaveUpper);
    gKnob5.ConsumeInput(gapTempo);
    gKnob6.ConsumeInput(gapKnob6);
    gKnob7.ConsumeInput(gapKnob7);

    AugSynthValueCategory category = (AugSynthValueCategory)gCatSelector.mValue;

    switch (gParamToSend)
    {
    case 0:
        SetAugSynthParam(category, 0, gKnob2.ToUnitFloat() *  1.032f);
        break;
    case 1:
        SetAugSynthParam(category, 1, gKnob3.ToUnitFloat() *  1.032f);
        break;
    case 2:
        SetAugSynthParam(category, 2, gKnob4.ToUnitFloat() *  1.032f);
        break;
    case 3:
        SetAugSynthParam(category, 3, gKnob5.ToUnitFloat() *  1.032f);
        break;
    case 4:
        SetAugSynthParam(category, 4, gKnob6.ToUnitFloat() *  1.032f);
        break;
    case 5:
        SetAugSynthParam(category, 5, gKnob7.ToUnitFloat() *  1.032f);
        break;
    }

    gParamToSend++;
    if (gParamToSend >= 6)
    {
        gParamToSend = 0;
    }
#endif // !AUG_SYNTH_DEBUG

    // Only want to apply settings while on general info.
    //if (gCurrScreenPage == ScreenPage::SP_GENERAL_INFO)
    {
        PedalMode nextPedalMode = (PedalMode)gPedalModeSelect.mValue;
        PedalMidiCh nextPedalMidiCh = (PedalMidiCh)gPedalMidiChSelect.mValue;
        PedalInternalParam nextPedalInternalParam = (PedalInternalParam)gPedalInternalParamSelect.mValue;

        if(nextPedalMode != gPedalMode || nextPedalMidiCh != gPedalMidiCh || nextPedalInternalParam != gPedalInternal)
        {
            ResetPedalForModeChange();
            gPedalMode = nextPedalMode;
            gPedalMidiCh = nextPedalMidiCh;
            gPedalInternal = nextPedalInternalParam;
        }

        UpdatePedal();
    }
}


/// @brief Play keys like a regular piano.
void PlayNotesDirect(uint8_t keyStart, uint8_t keyEnd)
{
    bool sustain = gdpSustainPedal.IsActive();
	for (uint8_t keyNum = keyStart; keyNum < keyEnd; keyNum++)
	{
		uint8_t vPinIdx = KeyNumToVirtualPin(keyNum);
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		
        NotePressInfo* pPressInfo = &gNoteStates[keyNum];
        
		NotePressState prevState = pPressInfo->mState;
		pPressInfo->ChangeState(vPinState, gTime, sustain);
		NotePressState currState = pPressInfo->mState;

		if (currState == NPS_PRESSED)
		{
			if (prevState == NPS_OFF)
			{
				SendNoteOn(keyNum);
			}
            else if(prevState == NPS_SUSTAINED)
            {
                SendNoteOff(keyNum);
                SendNoteOn(keyNum);
            }
		}
		else if(currState == NPS_OFF)
		{
			if (prevState == NPS_PRESSED || prevState == NPS_SUSTAINED)
			{
				SendNoteOff(keyNum);
			}
		}
	}
}



/// @brief Play the metronome
void PlayMetronome()
{
    // Tempo messages
    if (On4Note(24))
    {
        MIDI.sendClock();
    }

    bool noteOn = On4Note(1);
    bool noteOff = !noteOn && On4Note(8);
    uint8_t note = METRONOME_NOTE;

    if (noteOff && gPlayingMetronomeNote > 0)
    {
        gBpMsgBuff[0] = BP_CMD_CLICK_OFF;
        SendMessageToBp();
        gPlayingMetronomeNote = 0;
    }

    if (gdpMetronome.IsActive() && noteOn)
    {
        gBpMsgBuff[0] = BP_CMD_CLICK_ON;
        SendMessageToBp();
        gPlayingMetronomeNote = note;
    }
}

/// ===================================================================================
/// Utils
/// ===================================================================================

/// @brief Get the current pedal mode
PedalMode GetPedalMode()
{
    return (PedalMode)gPedalModeSelect.mValue;
}



/// @brief Get the pedal midi ch selection
PedalMidiCh GetPedalMidiCh()
{
    return (PedalMidiCh)gPedalMidiChSelect.mValue;
}



/// @brief Get pedal internal param
PedalInternalParam GetPedalInternalParam()
{
    return (PedalInternalParam)gPedalInternalParamSelect.mValue;
}



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



/// @brief Send a note on.
void SendNoteOn(uint8_t keyNum, uint8_t ch)
{
    uint8_t noteNum = KeyNumToNote(keyNum);
    uint8_t vel = ChannelIsPedal(ch) ? gPedalVelocity : DEFAULT_PLAY_VELOCITY;

#if LOOPER
    SendLooperNoteOn(noteNum, vel, ch);
#endif // LOOPER
    SendNoteOnMidi(noteNum, vel, ch);
}



/// @brief Send a note on.
void SendNoteOnMidi(uint8_t noteNum, uint8_t vel, uint8_t ch)
{
    if (ch == 0)
    {
        gBpMsgBuff[0] = BP_CMD_NOTE_ON;
        gBpMsgBuff[1] = noteNum;
        gBpMsgBuff[2] = vel;
        SendMessageToBp();
    }
    else
    {
        MIDI.sendNoteOn(noteNum, vel, ch);
    }
}



/// @brief Send a keynumber
void SendNoteOff(uint8_t keyNum)
{
    bool isUpper = IsUpperKey(keyNum);
    uint8_t ch = GetChannel(isUpper);

    SendNoteOff(keyNum, ch);
}



/// @brief Send a note off.
void SendNoteOff(uint8_t keyNum, uint8_t ch)
{
    uint8_t noteNum = KeyNumToNote(keyNum);

#if LOOPER
    SendLooperNoteOff(noteNum, ch);
#endif // LOOPER
    SendNoteOffMidi(noteNum, ch);
}



/// @brief Send a note off.
void SendNoteOffMidi(uint8_t noteNum, uint8_t ch)
{
    if (ch == 0)
    {
        gBpMsgBuff[0] = BP_CMD_NOTE_OFF;
        gBpMsgBuff[1] = noteNum;
        gBpMsgBuff[2] = 0;
        SendMessageToBp();
    }
    else
    {
        MIDI.sendNoteOff(noteNum, 0, ch);
    }
}



/// @brief Cancel note on every channel
void SendNoteOffAllCh()
{
    for(uint8_t ch = 0; ch <= 16; ch++)
    {
        MIDI.sendControlChange(midi::MidiControlChangeNumber::AllNotesOff, 0, ch);
        MIDI.sendControlChange(midi::MidiControlChangeNumber::AllSoundOff, 0, ch);
    }

    // Mute BP synth
    ResetBpSynth();
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

/// ===================================================================================
/// Pedal
/// ===================================================================================

/// @brief Send a CC message for the pedal's value
void SendPedalMidiCC(uint8_t ccMode, uint8_t value)
{
    switch (gPedalMidiCh)
    {
    case PMC_LOWER:
        MIDI.sendControlChange(ccMode, value, gLowerCh.mValue);
        break;
    case PMC_UPPER:
        MIDI.sendControlChange(ccMode, value, gUpperCh.mValue);
        break;
    case PMC_UPPER_LOWER:
        MIDI.sendControlChange(ccMode, value, gUpperCh.mValue);
        MIDI.sendControlChange(ccMode, value, gLowerCh.mValue);
        break;
    default:
        uint8_t ch = (gPedalMidiCh + 1) - (PMC_MIDI_CH1);
        MIDI.sendControlChange(ccMode, value, ch);
        break;
    }
}

/// @brief Send a pitch bend message for the pedal's value
void SendPedalPitchBend(int value)
{
    switch (gPedalMidiCh)
    {
    case PMC_LOWER:
        MIDI.sendPitchBend(value, gLowerCh.mValue);
        break;
    case PMC_UPPER:
        MIDI.sendPitchBend(value, gUpperCh.mValue);
        break;
    case PMC_UPPER_LOWER:
        MIDI.sendPitchBend(value, gUpperCh.mValue);
        MIDI.sendPitchBend(value, gLowerCh.mValue);
        break;
    default:
        uint8_t ch = (gPedalMidiCh + 1) - (PMC_MIDI_CH1);
        MIDI.sendPitchBend(value, ch);
        break;
    }
}

/// @brief Convert PIP enum to param index
uint8_t PIPToParamNum(PedalInternalParam pipVal)
{
    switch (pipVal)
    {    
        case PIP_GAIN:
            return ASP_GAIN;
        case PIP_DELAY_TIME:
            return ASP_DELAY_TIME;
        case PIP_DCO_VOL_1:
            return ASP_DCO_VOL_1;
        case PIP_DCO_VOL_2:
            return ASP_DCO_VOL_2;
        case PIP_DCO_TUNE_1:
            return ASP_DCO_TUNE_1;
        case PIP_DCO_TUNE_2:
            return ASP_DCO_TUNE_2;
        case PIP_DCO_SHAPE_1:
            return ASP_DCO_WS_1;
        case PIP_DCO_SHAPE_2:
            return ASP_DCO_WS_2; 
        case PIP_CUTOFF:
            return ASP_VCF_CUTOFF;
        case PIP_LFO_RATE:
            return ASP_LFO_RATE;
        case PIP_LFO_OSC1_FREQ:
            return ASP_LFO_OSC1_TUNE;
        case PIP_LFO_OSC2_FREQ:
            return ASP_LFO_OSC2_TUNE;
        case PIP_LFO_OSC1_VOL:
            return ASP_LFO_OSC1_VOLUME;
        case PIP_LFO_OSC2_VOL:
            return ASP_LFO_OSC2_VOLUME;
        default:
            break;
    }

    return ASP_GAIN;
}

void SendPedalInternal()
{
    uint8_t paramIdx = PIPToParamNum(gPedalInternal);
    gpPedalInternalParam = &gAugSynthParams[paramIdx];

    float pedalValueF = GetPedalStable() * (1.0f / (float)ANALOG_MAX_VALUE);
    if(paramIdx == ASP_DCO_TUNE_1 || paramIdx == ASP_DCO_TUNE_2)
    {
        pedalValueF -= 0.5f;
        pedalValueF *= 2.0f;
    }

    pedalValueF = AugSynthParam::ScaleFloat(paramIdx, pedalValueF);
    SendParameterToBp(paramIdx, pedalValueF);
}

/// @brief Call this before changing any pedal parameters
void ResetPedalForModeChange()
{
    switch (gPedalMode)
    {
        case PM_VELOCITY:
            gPedalVelocity = DEFAULT_PLAY_VELOCITY;
            break;
        case PM_PITCH_BEND:
            SendPedalPitchBend(0);
            break;
        case PM_MODULATION:
            SendPedalMidiCC(midi::MidiControlChangeNumber::ModulationWheel , 0);
            break;
        case PM_VOLUME:
            SendPedalMidiCC(midi::MidiControlChangeNumber::ExpressionController, 127);
            break;
        case PM_INTERNAL:
            if(gpPedalInternalParam)
            {
                gpPedalInternalParam->SendValueToBpSynth();
            }
            gpPedalInternalParam = nullptr;
            break;
        default:
            break;
    }
}

/// @brief Is this channel the pedal's channel?
bool ChannelIsPedal(uint8_t ch)
{
    if(gPedalMode == PM_OFF || gPedalMode == PM_INTERNAL)
    {
        return false;
    }

    switch (gPedalMidiCh)
    {
    case PMC_LOWER:
        return ch == gLowerCh.mValue;
    case PMC_UPPER:
        return ch == gUpperCh.mValue;
    case PMC_UPPER_LOWER:
        return ch == gUpperCh.mValue || ch == gLowerCh.mValue;
    default:
        uint8_t pedalCh = (gPedalMidiCh + 1) - (PMC_MIDI_CH1);
        return ch == pedalCh;
    }
}

void UpdatePedal()
{
    uint32_t pedalVal = GetPedalStable();
    uint8_t pedal7 = (uint8_t)(min(pedalVal >> 3, 127)); // 10 bits -> 7
    int pitchBend;

    switch (gPedalMode)
    {
        case PM_VELOCITY:
            gPedalVelocity = pedal7;
            break;
        case PM_PITCH_BEND:
            pitchBend = (int)(pedalVal << 4) - MIDI_PITCHBEND_MAX; // 10 bits -> 14 bits
            pitchBend = max(min(pitchBend, MIDI_PITCHBEND_MAX), MIDI_PITCHBEND_MIN);
            SendPedalPitchBend(pitchBend);
            break;
        case PM_MODULATION:
            SendPedalMidiCC(midi::MidiControlChangeNumber::ExpressionController, pedal7);
            break;
        case PM_VOLUME:
            SendPedalMidiCC(midi::MidiControlChangeNumber::ExpressionController, pedal7);
            break;
        case PM_INTERNAL:
            SendPedalInternal();
            break;
        case PM_OFF:
            break;
        default:
            break;
    }
}

/// ===================================================================================
/// Aug Synth
/// ===================================================================================

/// @brief Send a message to the BP synth
void ResetBpSynth()
{
    constexpr uint8_t BUFF_SIZE = sizeof(gBpMsgBuff) / sizeof(uint8_t);

    // Writing 0 into the header will reset the bp synth
    for (uint8_t i = 0; i < BUFF_SIZE+1; i++)
    {
        Serial1.write(0);
    }
    PollRotaryEncoders();
}

/// @brief Send a message to the BP synth
void SendMessageToBp()
{
    constexpr uint8_t BUFF_SIZE = sizeof(gBpMsgBuff) / sizeof(uint8_t);

    Serial1.write(BP_CMD_HEADER);
    for (uint8_t i = 0; i < BUFF_SIZE; i++)
    {
        Serial1.write(gBpMsgBuff[i]);
        
    }
    delayMicroseconds(50); // let bp interupt finish

    PollRotaryEncoders();
}


/// @brief Send a message to the BP synth
void SendParameterToBp(const uint8_t paramNum, const float value)
{
    gBpMsgBuff[0] = paramNum & 0x7F;

    float* floatPtr = reinterpret_cast<float*>(gBpMsgBuff + 1);
    *floatPtr = value;

    SendMessageToBp();
}

void SendParameterToBp(const uint8_t paramNum, const uint8_t value)
{
    gBpMsgBuff[0] = paramNum & 0x7F;
    uint32_t* uint32Ptr = reinterpret_cast<uint32_t*>(gBpMsgBuff + 1);
    *uint32Ptr = (uint32_t)value;

    SendMessageToBp();
}

#if AUG_SYNTH_DEBUG
/// @brief Set a parameter on the aug synth
/// @param category Category
/// @param index Param num in category
/// @param value Value to set
void SetAugSynthParam(const AugSynthValueCategory category, const uint8_t index, const float value)
{
    switch (category)
    {
    case AugSynthValueCategory::General:
        switch (index)
        {
        case 0:
            SendMessageToBp(ASP_TUNING, value);
            break;
        case 1:
            SendMessageToBp(ASP_DRIVE, value);
            break;
        case 2:
            SendMessageToBp(ASP_GAIN, value);
            break;
        case 3:
            SendMessageToBp(ASP_DELAY_TIME, value);
            break;
        case 4:
            SendMessageToBp(ASP_DELAY_FEEDBACK, value);
            break;
        case 5:
            SendMessageToBp(ASP_DELAY_SHEAR, value);
            break;
        }
        break;
    case AugSynthValueCategory::DCO:
        switch (index)
        {
        case 0:
            SendMessageToBp(ASP_DCO_WAVE_SHAPE_1, value);
            break;
        case 1:
            SendMessageToBp(ASP_DCO_TUNE_1, value);
            break;
        case 2:
            SendMessageToBp(ASP_DCO_VOL_1, value);
            break;
        case 3:
            SendMessageToBp(ASP_DCO_WAVE_SHAPE_2, value);
            break;
        case 4:
            SendMessageToBp(ASP_DCO_TUNE_2, value);
            break;
        case 5:
            SendMessageToBp(ASP_DCO_VOL_2, value);
            break;
        }
        break;
    case AugSynthValueCategory::VCF:
        switch (index)
        {
        case 0:
            SendMessageToBp(ASP_VCF_CUTOFF, value);
            break;
        case 1:
            SendMessageToBp(ASP_VCF_RES, value);
            break;
        case 2:
            SendMessageToBp(ASP_VCF_MODE, value);
            break;
        case 3:
            SendMessageToBp(ASP_VCF_CUTOFF_LFO, value);
            break;
        case 4:
            SendMessageToBp(ASP_VCF_RES_LFO, value);
            break;
        case 5:
            SendMessageToBp(ASP_VCF_ENV1, value);
            break;
        }
        break;
    case AugSynthValueCategory::LFO:
        switch (index)
        {
        case 0:
            SendMessageToBp(ASP_LFO_RATE, value);
            break;
        case 1:
            SendMessageToBp(ASP_LFO_WAVE_SHAPE, value);
            break;
        case 2:
            SendMessageToBp(ASP_LFO_ATTACK, value);
            break;
        case 3:
            SendMessageToBp(ASP_LFO_GAIN, value);
            break;
        case 4:
            SendMessageToBp(ASP_LFO_OSC1_TUNE, value);
            break;
        case 5:
            SendMessageToBp(ASP_LFO_OSC2_TUNE, value);
            break;
        }
        break;
    case AugSynthValueCategory::ENV:
        switch (index)
        {
        case 0:
            SendMessageToBp(ASP_ENV_ATTACK1, value);
            break;
        case 1:
            SendMessageToBp(ASP_ENV_SUSTAIN1, value);
            break;
        case 2:
            SendMessageToBp(ASP_ENV_DECAY1, value);
            break;
        case 3:
            SendMessageToBp(ASP_ENV_ATTACK2, value);
            break;
        case 4:
            SendMessageToBp(ASP_ENV_SUSTAIN2, value);
            break;
        case 5:
            SendMessageToBp(ASP_ENV_DECAY2, value);
            break;
        }
        break;
    }
}
#endif // AUG_SYNTH_DEBUG