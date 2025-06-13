#if 1
#include <Arduino.h>

#ifndef LOOPER_H
#define LOOPER_H

struct LooperEventNoteOn
{
    uint8_t mNote;
    uint8_t mVelocity;
    uint8_t mCh;
    uint16_t mTime;
};

struct LooperEventNoteOff
{
    uint8_t mNote;
    uint8_t mCh;
    uint16_t mTime;
};

struct LooperRecordingPass // __attribute__((packed))
{
    uint16_t mBegin;
    uint16_t mEnd;
    uint16_t mPlaybackOffset;
    bool mIsActionBegin;
    uint8_t mRecCh;

    LooperRecordingPass() : 
        mBegin(0),
        mEnd(0),
        mPlaybackOffset(0),
        mIsActionBegin(false),
        mRecCh(0)
    {
    }
};

void InitLooper();
void EraseAllLoops();

uint8_t GetPedalsRecCh();
void SendLooperNoteOn(uint8_t note, uint8_t vel, uint8_t midiCh);
void SendLooperNoteOff(uint8_t note, uint8_t midiCh);

void UpdateLooper();

#endif // LOOPER_H
#endif // LOOPER