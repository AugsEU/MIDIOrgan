#include <Arduino.h>
#include <TimeInfo.h>

#ifndef ARP_H
#define ARP_H

enum ArpMode
{
    ARP_OFF,
    ARP_RND,
    ARP_UP,
    ARP_UP_SPEC,
    ARP_DOWN,
    ARP_DOWN_SPEC,
    ARP_UP_DOWN,
    ARP_UP_DOWN_SPEC,
};

struct Arp
{
    ArpMode mMode = ARP_OFF;

    //void PlayNotes();
};

#endif // ARP_H