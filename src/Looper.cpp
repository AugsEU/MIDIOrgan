#if 1
/// ===================================================================================
/// Include
/// ===================================================================================
#include <Looper.h>
#include <Util/TimeInfo.h>
#include <UserControls.h>
#include <Tempo.h>
#include <Util/Globals.h>
#include <Util/Debug.h>
#include <MidiOutput.h>
#include <Util/AugMath.h>
#include <Util/Utils.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr uint16_t TIME_BIT_SHIFT = 1; // 0 = 1ms resolution, 1 = 2ms, 3 = 4ms, and so on...
constexpr uTimeMs MAX_LOOP_TIME = ((1ul << 16ul)-1ul) * (1ul << (uTimeMs)TIME_BIT_SHIFT); // Max time in ms of looper.
constexpr size_t LOOPER_BYTES = 2048;
constexpr size_t MAX_REC_PASSES = 64;
constexpr uint8_t NOTE_ON_EVENT_SIZE = sizeof(LooperEventNoteOn) * sizeof(uint8_t);
constexpr uint8_t NOTE_OFF_EVENT_SIZE = sizeof(LooperEventNoteOff) * sizeof(uint8_t);

/// ===================================================================================
/// Globals
/// ===================================================================================
uint8_t gEventBuff[LOOPER_BYTES]; // huge buffer.
uint16_t gEventWriteHead = 0;
uint8_t gNumPendingNoteOffs = 0;

LooperRecordingPass gRecPasses[MAX_REC_PASSES];
LooperRecordingPass* gCurrRecPass;
uint8_t gRecPassesCount = 0;

bool gLoopLengthSet = false;
uTimeMs gCurrLoopLength = 0;
uTimeMs gCurrLoopPlayHead = 0;

/// ===================================================================================
/// Private fn
/// ===================================================================================
LooperRecordingPass* GetTopRecordingPass();

/// ===================================================================================
/// Setup
/// ===================================================================================

/// @brief Called at startup
void InitLooper()
{
    EraseAllLoops();
}

void EraseAllLoops()
{
    if(gLoopLengthSet && gEventWriteHead > 0) // Mute if we have notes.
    {
        // @TODO more fine grain muting.
        SendNoteOffAllCh();
    }

    gEventWriteHead = 0;
    gRecPassesCount = 0;
    gCurrLoopLength = 0;
    gNumPendingNoteOffs = 0;
    gCurrRecPass = nullptr;

    gLoopLengthSet = false;

    for(size_t i = 0; i < MAX_REC_PASSES; i++)
    {
        gRecPasses[i] = LooperRecordingPass();
    }

    // In theory we don't need this, but just in case.
    for(size_t i = 0; i < LOOPER_BYTES; i++)
    {
        gEventBuff[i] = 0;
    }
}

/// ===================================================================================
/// Buffer
/// ===================================================================================

/// @brief Writes a note on event to the buffer
bool TryWriteNoteOnEvent(uint8_t note, uint8_t vel, uint8_t ch)
{
    if((gEventWriteHead + NOTE_ON_EVENT_SIZE + NOTE_OFF_EVENT_SIZE * gNumPendingNoteOffs) > LOOPER_BYTES)
    {
        // We won't have space to write the corresponding note off.
        return false;
    }

    uint16_t time = (uint16_t)(gCurrLoopPlayHead >> TIME_BIT_SHIFT);
    gEventBuff[gEventWriteHead++] = note | 0x80; // Write leading 1 to indicate this is note on.
    gEventBuff[gEventWriteHead++] = vel;
    gEventBuff[gEventWriteHead++] = ch;
    WriteU16ToByteBuff(gEventBuff, gEventWriteHead, time);

    DebugPrint("WNote on:", note);

    gNumPendingNoteOffs++;
    return true;
}

/// @brief Read a note on event from the buffer
LooperEventNoteOn ReadNoteOnEvent(uint16_t idx)
{
    LooperEventNoteOn ret;

    ret.mNote = gEventBuff[idx++] & 0x7F;
    ret.mVelocity = gEventBuff[idx++];
    ret.mCh = gEventBuff[idx++];
    ret.mTime = ReadU16FromByteBuff(gEventBuff, idx);

    return ret;
}

/// @brief Writes a note on event to the buffer
bool TryWriteNoteOffEvent(uint8_t note, uint8_t ch)
{
    if(gEventWriteHead + NOTE_OFF_EVENT_SIZE > LOOPER_BYTES)
    {
        // We won't have space to write the note off. Something has gone wrong.
        AG_ASSERT(false, 100);
        EraseAllLoops();
        return false;
    }

    uint16_t time = (uint16_t)(gCurrLoopPlayHead >> TIME_BIT_SHIFT);
    gEventBuff[gEventWriteHead++] = note & 0x7F; // Write leading 0 to indicate this is note off.
    gEventBuff[gEventWriteHead++] = ch;
    *(uint16_t*)(gEventBuff + gEventWriteHead) = time;
    gEventWriteHead += sizeof(uint16_t) * sizeof(uint8_t);

    DebugPrint("WNote off:", note);

    if(gNumPendingNoteOffs > 0)
    {
        gNumPendingNoteOffs--;
    }
    return true;
}

/// @brief Read a note off event from the buffer
LooperEventNoteOff ReadNoteOffEvent(uint16_t idx)
{
    LooperEventNoteOff ret;

    ret.mNote = gEventBuff[idx++] & 0x7F;
    ret.mCh = gEventBuff[idx++];
    ret.mTime = *reinterpret_cast<uint16_t*>(gEventBuff + idx);

    return ret;
}

/// ===================================================================================
/// Recording
/// ===================================================================================

/// @brief Start recording in channel.
void BeginRecordingInChannel(uint8_t ch)
{
    if(gRecPassesCount >= MAX_REC_PASSES)
    {
        return; // No space for pass.
    }
    gRecPassesCount++;
    LooperRecordingPass* pass = GetTopRecordingPass();
    if(pass == nullptr)
    {
        AG_ASSERT(pass != nullptr, 102);
        EraseAllLoops();
        return;
    }

    pass->mBegin = gEventWriteHead;
    pass->mRecCh = ch;
    pass->mIsActionBegin = true;
    pass->mPlaybackOffset = 0;

    DebugPrint("Begin ch", ch);
    gCurrRecPass = pass;
    
    for(int i = 0; i < NUM_NOTES; i++)
    {
        gNoteStates[i].mState = NPS_OFF;
    }
}

/// @brief Need to create a new pass to extend loop.
void ExtendRecordingInChannel(uint8_t ch)
{
    gRecPassesCount++;
    LooperRecordingPass* pass = GetTopRecordingPass();
    if(pass == nullptr)
    {
        // @TODO Mute all notes
        return; // No space.
    }

    pass->mBegin = gEventWriteHead;
    pass->mRecCh = ch;
    pass->mIsActionBegin = false;
    pass->mPlaybackOffset = 0;

    gCurrRecPass = pass;
}

/// @brief Get the current channel we are recording on. Zero means not recording.
uint8_t GetPedalsRecCh()
{
    if(gdpLoop1Rec.IsActive())
    {
        return 1;
    }
    else if(gdpLoop2Rec.IsActive())
    {
        return 2;
    }
    else if(gdpLoop3Rec.IsActive())
    {
        return 3;
    }
    else if(gdpLoop4Rec.IsActive())
    {
        return 4;
    }

    return 0;
}

/// @brief Get the "pass" at the top of the stack.
LooperRecordingPass* GetTopRecordingPass()
{
    if (gRecPassesCount == 0 || gRecPassesCount > MAX_REC_PASSES)
    {
        return nullptr;
    }

    return &gRecPasses[gRecPassesCount-1];
}


/// @brief Notify the looper that a note has been pressed
void SendLooperNoteOn(uint8_t note, uint8_t vel, uint8_t midiCh)
{
    LooperRecordingPass* pass = gCurrRecPass;
    if(pass == nullptr || pass->mRecCh == 0)
    {
        return; // Reached pass limit or pass not recording.
    }

    if(TryWriteNoteOnEvent(note, vel, midiCh))
    {
        // Event written, update end.
        pass->mEnd = gEventWriteHead;
    }
}

/// @brief Notify the looper that a note has been released
void SendLooperNoteOff(uint8_t note, uint8_t midiCh)
{
    LooperRecordingPass* pass = gCurrRecPass;
    if(pass == nullptr || pass->mRecCh == 0)
    {
        return; // Reached pass limit or pass not recording.
    }

    if(TryWriteNoteOffEvent(note, midiCh))
    {
        // Event written, update end.
        pass->mEnd = gEventWriteHead;
    }
}

/// ===================================================================================
/// Playback
/// ===================================================================================

void PlayPassNotes(LooperRecordingPass* pass)
{
    uint16_t time = (uint16_t)gCurrLoopPlayHead >> TIME_BIT_SHIFT;
    while(true)
    {
        uint16_t idx = pass->mBegin + pass->mPlaybackOffset;
        if(idx >= pass->mEnd || idx >= LOOPER_BYTES)
        {
            break;
        }

        if((gEventBuff[idx] & 0x80) != 0) // Note on
        {
            LooperEventNoteOn noteOn = ReadNoteOnEvent(idx);
            if(noteOn.mTime < time)
            {
                DebugPrint("PNote on:", noteOn.mNote);
                SendNoteOnMidi(noteOn.mNote, noteOn.mVelocity, noteOn.mCh);
                pass->mPlaybackOffset += NOTE_ON_EVENT_SIZE;
            }
            else
            {
                break;
            }
        }
        else
        {
            LooperEventNoteOff noteOff = ReadNoteOffEvent(idx);
            if(noteOff.mTime < time)
            {
                DebugPrint("PNote off:", noteOff.mNote);
                SendNoteOffMidi(noteOff.mNote, noteOff.mCh);
                pass->mPlaybackOffset += NOTE_OFF_EVENT_SIZE;
            }
            else
            {
                break;
            }
        }
    }
}

/// @brief Play all recorded notes.
void PlayRecordedNotes()
{
    for(uint8_t i = 0; i < gRecPassesCount; i++)
    {
        LooperRecordingPass* pass = &gRecPasses[i];
        if(pass != gCurrRecPass)
        {
            PlayPassNotes(pass);
        }
    }
}

/// ===================================================================================
/// Update
/// ===================================================================================

/// @brief Try to start the recording for the first pass.
void TryStartRecording(uint8_t recCh)
{
    if(recCh == 0)
    {
        return; // Not recording.
    }

    if(!(CloseToBeatStart() || On4Note(1)))
    {
        return; // Wait until beat start.
        // Note: is is possible for us to not be close to the beat start but still be on a quarter note at a high tempo.
    }

    // OK to start recording.
    BeginRecordingInChannel(recCh);

    gCurrLoopLength = TimeSinceBeat();
}

bool ShouldStopRecording()
{
    uint8_t recCh = GetPedalsRecCh();
    if(recCh != 0)
    {
        return false; // Footswitch still held.
    }

    if(gNumPendingNoteOffs > 0)
    {
        return false; // Notes still held.
    }

    return true;
}

/// @brief Called every tick
void UpdateLooper()
{
    if(gdpLoopClear.IsActive())
    {
        EraseAllLoops();
    }
    
    uTimeMs dt = gTime - gPrevTime;
    uint8_t pressedCh = GetPedalsRecCh();

    if(gRecPassesCount == 0)
    {
        TryStartRecording(pressedCh);
    }
    else if(gCurrRecPass) // Recording a pass
    {
        bool endRec = ShouldStopRecording();
        if(!gLoopLengthSet) // Special case: Recording first pass.
        {
            gCurrLoopLength += dt;
            gCurrLoopPlayHead = gCurrLoopLength;
            if(gCurrLoopLength > MAX_LOOP_TIME)
            {
                // Loop too long. @TODO Properly exit from this condition.
                EraseAllLoops();
                return;
            }

            if(pressedCh == 0)
            {
                gCurrLoopLength = RoundToNearestMultiple(gCurrLoopLength, gTempoInterval); // Round to nearest bpm.
                if(gCurrLoopLength < gTempoInterval * 2)
                {
                    EraseAllLoops();
                    return;
                }
                AG_ASSERT(gCurrLoopLength % gTempoInterval == 0, 101);
                gCurrLoopPlayHead = TimeSinceBeat(); // @TODO think about this creating a small offset.
                gLoopLengthSet = true;
            }
        }

        if(endRec)
        {
            // @TODO Do something to end the pass?
            DebugPrint("End rec", gCurrLoopLength);
            gCurrRecPass = nullptr;
        }
    }
    else if(pressedCh != 0)
    {
        BeginRecordingInChannel(pressedCh);
    }


    if(gLoopLengthSet)
    {
        gCurrLoopPlayHead += dt;
        if(gCurrLoopPlayHead >= gCurrLoopLength) // Wrap around.
        {
            gCurrLoopPlayHead -= gCurrLoopLength;
            if(gCurrRecPass) // Pass is only non-null if we are recording.
            {
                ExtendRecordingInChannel(gCurrRecPass->mRecCh);
            }
            for(uint8_t i = 0; i < gRecPassesCount; i++)
            {
                gRecPasses[i].mPlaybackOffset = 0;
            }
        }

        PlayRecordedNotes();
    }

    // Lock tempo when looping.
    SetTempoLock(gRecPassesCount > 0);
}
#endif // LOOPER