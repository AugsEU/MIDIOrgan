#include <Tempo.h>
#include <Globals.h>

uTimeMs gTempoInterval; // Quarter note interval in milliseconds

/// @brief Set tempo in bpm
void SetTempo(uint16_t tempo)
{
    gTempoInterval = 1000 * 60 / tempo;
}


/// @brief Did we just pass a quarter note?
bool On4Note(uint8_t division)
{
    unsigned long prevQuarterNote = (gPrevTime * division) / gTempoInterval;
    unsigned long nowQuarterNote = (gTime * division) / gTempoInterval;

    return nowQuarterNote > prevQuarterNote;
}