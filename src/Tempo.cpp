#include <Tempo.h>
#include <Globals.h>
#include <UserControls.h>
#include <StableAnalog.h>

uTimeMs gTempoInterval = 500; // Quarter note interval in milliseconds
uTimeMs gTempoTicker = 0;
StableAnalog gTempoReading;

/// @brief Set tempo in bpm
void SetTempo(uint16_t tempo)
{
    gTempoInterval = (1000ul * 60ul) / tempo;
}

static_assert(ANALOG_MAX_VALUE == 1024, "Need to change function below when changing analog max value");
/// @brief Calculates tempo from analog values.
uint32_t CalculateTempo(uint32_t x)
{
    // Calculate (640 * x^3) / (1024^3) without overflow:
    constexpr uint32_t divisor = (uint32_t)ANALOG_MAX_VALUE * (uint32_t)ANALOG_MAX_VALUE;
    uint32_t term = x * x * 640 / divisor; 
    term *= x;
    term /= (uint32_t)ANALOG_MAX_VALUE;

    uint32_t tempo = 40 + term;

    return tempo;
}


/// @brief Update tempo
void UpdateTempo()
{
    gTempoReading.ConsumeInput(gapTempo);
    if (gTime <= gPrevTime)
    {
        return;
    }

    gTempoTicker += (gTime - gPrevTime);

    if (gTempoTicker > gTempoInterval) // End of beat.
    {
        gTempoTicker -= gTempoInterval;

        SetTempo(CalculateTempo(gTempoReading.mStableValue)); // Calculate new tempo
    }
}


/// @brief Did we just pass a quarter note?
bool On4Note(uint8_t division)
{
    uTimeMs dt = gTime - gPrevTime;

    uTimeMs spedUpTicker = (gTempoTicker * division) % gTempoInterval;

    return spedUpTicker < dt * division; 
}

/// @brief Did we just pass a quarter note?
bool On3Note(uint8_t division)
{
    return false; // TODO: Do we need this?
}