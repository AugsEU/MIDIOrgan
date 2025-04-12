#include <Tempo.h>
#include <Globals.h>

uTimeMs gTempoInterval; // Quarter note interval in milliseconds

/// @brief Set tempo in bpm
void SetTempo(uint16_t tempo)
{
    gTempoInterval = 1000 * 60 / tempo;
}


/// @brief Did we just pass a quarter note?
bool On4Note()
{
    unsigned long prevQuarterNote = gPrevTime / gTempoInterval;
    unsigned long nowQuarterNote = gTime / gTempoInterval;

    return nowQuarterNote > prevQuarterNote;
}


/// @brief Did we just pass an eigth note?
bool On8Note()
{
    unsigned long prevEightNote = (gPrevTime * 2) / gTempoInterval;
    unsigned long nowEightNote = (gTime * 2) / gTempoInterval;

    return nowEightNote > prevEightNote;
}