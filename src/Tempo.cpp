#include <Tempo.h>

uTimeMs gTempoInterval; // Quarter note interval in milliseconds

/// @brief Set tempo in bpm
void SetTempo(uint16_t tempo)
{
    gTempoInterval = 1000 * 60 / tempo;
}


/// @brief Did we just pass a quarter note?
bool On4Note(uTimeMs prevTime, uTimeMs time)
{
    unsigned long prevQuarterNote = prevTime / gTempoInterval;
    unsigned long nowQuarterNote = time / gTempoInterval;

    return nowQuarterNote > prevQuarterNote;
}


/// @brief Did we just pass an eigth note?
bool On8Note(uTimeMs prevTime, uTimeMs time)
{
    unsigned long prevEightNote = (prevTime * 2) / gTempoInterval;
    unsigned long nowEightNote = (time * 2) / gTempoInterval;

    return nowEightNote > prevEightNote;
}