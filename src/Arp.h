#include <Arduino.h>
#include <TimeInfo.h>

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

bool ArpEnabled();
void ReadArpMode();
void PlayArp();

#endif // ARP_H