#include <Arduino.h>
#include <Util/TimeInfo.h>
#include <Util/Constants.h>

#ifndef ARP_H
#define ARP_H

enum ArpMode : uint8_t
{
    ARP_OFF         = 0,
    ARP_UP          = 1,
    ARP_DOWN        = (1 << 1),
    ARP_SPEC        = (1 << 2),
    ARP_UP_SPEC     = ARP_UP | ARP_SPEC,
    ARP_DOWN_SPEC   = ARP_DOWN | ARP_SPEC,
    ARP_UP_DOWN     = ARP_UP | ARP_DOWN,
    ARP_UP_DOWN_SPEC= ARP_UP | ARP_DOWN | ARP_SPEC,
};

enum ArpSpeed : uint8_t
{
    ARP_SPEED_EIGHTH = 0b00,
    ARP_SPEED_SIXTEENTH = 0b10,
    ARP_SPEED_QUARTER = 0b01,
    ARP_SPEED_TRIPLET = 0b11
};

struct Arpeggiator
{
    uint8_t mPlayingKey = NOTE_NONE;
    bool mGoingUp;
    uTimeMs mLastNoteTime = 0;
    uTimeMs mPressNoteTime = 0;

    void PlayNotes(uint8_t keyStart, uint8_t keyEnd);
    void ResetState();

private:
    void ChooseNextNote(uint8_t lowestKey, uint8_t highestKey);
};

bool ArpEnabled();
void ReadArpMode();
void PlayArp();
void ArpTimerOverflowFixup();

#endif // ARP_H