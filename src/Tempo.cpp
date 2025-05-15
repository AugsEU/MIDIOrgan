#include <Tempo.h>
#include <Util/Globals.h>
#include <UserControls.h>
#include <Input/StableAnalog.h>

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

    if (x <= 512)
    {
        // 40 + 320x / 1024 - 320xx / (1024*1024);
        x *= 320 * (1024 - x);
        x >>= 20;
        gTempoCache = 41 + x;
    }
    else
    {
        // 632 - 2x + xx / 512
        x = x * x - 1024*x;
        x >>= 9;
        gTempoCache = 632 + x;
    }
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