#include <Tempo.h>
#include <Globals.h>
#include <UserControls.h>
#include <StableAnalog.h>

uTimeMs gTempoInterval = 500; // Quarter note interval in milliseconds
uTimeMs gTempoTicker = 0;
uint16_t gTempoCache = 120;
StableAnalog gTempoReading;

/// @brief Set tempo in bpm
void SetTempo()
{
    gTempoInterval = (1000ul * 120ul) / gTempoCache; // 2 beats
}

static_assert(ANALOG_MAX_VALUE == 1024, "Need to change function below when changing analog max value");
/// @brief Calculates tempo from analog values.
void RecalculateTempo()
{
    uint32_t x = gTempoReading.mStableValue;
    // Calculate (640 * x^3) / (1024^3) without overflow:
    constexpr uint32_t divisor = (uint32_t)ANALOG_MAX_VALUE * (uint32_t)ANALOG_MAX_VALUE;
    uint32_t term = x * x * 640 / divisor; 
    term *= x;
    term /= (uint32_t)ANALOG_MAX_VALUE;

    gTempoCache = 40 + term;
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

        RecalculateTempo(); // Calculate new tempo
        SetTempo(); // Set new tempo
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