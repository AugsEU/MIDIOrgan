#include <AugSynthPreset.h>
#include <Util/Utils.h>
#include <MidiOutput.h>

/// ===================================================================================
/// Constants
/// ===================================================================================

constexpr AugSynthPreset gFactoryPresets[NUM_FACTORY_PRESETS] PROGMEM = {
    {{'I','n','i','t',' ',' ',' ',' ',}, {
        TUNING_12TET, 0, 20,            // Tune Drive Gain 
        10, 0, 0, DELAY_MODE_OFF,       // Delay: Time Fbck Shear Mode
        OSC_MODE_SINE, 0, 50, 5,        // Osc1: Wave Tune Vol Shape
        OSC_MODE_SINE, 0, 0, 5,         // Osc2: Wave Tune Vol Shape
        2, 10, 40, 5,                   // Env1: A D S R
        2, 10, 40, 5,                   // Env2: A D S R
        20, 0, FILTER_MODE_OFF, 0,      // Filt: Freq Res Mode Env
        25, OSC_MODE_SINE,  0, 0,       // LFO: Rate Mode Atk Wbbl
        0, 0, 0,                        // LOsc1: Tune Vol Shp
        0, 0, 0,                        // LOsc2: Tune Vol Shp
        0, 0,                           // LVCF: Freq Res
        SOUND_TYPE_POLY                 // Sound Type
    }},
    {{'L','e','s','l','i','e',' ',' ',}, {
        TUNING_12TET, 0, 20,            // Tune Drive Gain 
        20, 3, 0, DELAY_MODE_NORMAL,    // Delay: Time Fbck Shear Mode
        OSC_MODE_ORGAN, 0, 50, 2,       // Osc1: Wave Tune Vol Shape
        OSC_MODE_ORGAN, 50, 40, 10,     // Osc2: Wave Tune Vol Shape
        1, 5, 40, 1,                    // Env1: A D S R
        1, 5, 37, 1,                    // Env2: A D S R
        40, 5, FILTER_MODE_LP, 0,       // Filt: Freq Res Mode Env
        30, OSC_MODE_SINE,  0, 3,       // LFO: Rate Mode Atk Wbbl
        0, 3, -2,                       // LOsc1: Tune Vol Shp
        1, -8, 20,                      // LOsc2: Tune Vol Shp
        -2, 0,                           // LVCF: Freq Res
        SOUND_TYPE_POLY                 // Sound Type
    }},
    {{'F','u','n','k','B','a','s','s',}, {
        TUNING_12TET, 0, 20,            // Tune Drive Gain 
        0, 0, 0, DELAY_MODE_OFF,        // Delay: Time Fbck Shear Mode
        OSC_MODE_SAW, -40, 50, 2,       // Osc1: Wave Tune Vol Shape
        OSC_MODE_SQUARE, -40, 50, 10,   // Osc2: Wave Tune Vol Shape
        0, 6, 40, 10,                   // Env1: A D S R
        0, 2, 40, 15,                   // Env2: A D S R
        9, 20, FILTER_MODE_LP, 17,      // Filt: Freq Res Mode Env
        25, OSC_MODE_SINE,  0, 0,       // LFO: Rate Mode Atk Wbbl
        0, 0, 0,                        // LOsc1: Tune Vol Shp
        0, 0, 0,                        // LOsc2: Tune Vol Shp
        0, 0,                           // LVCF: Freq Res
        SOUND_TYPE_BASS                 // Sound Type
    }},
};

/// ===================================================================================
/// Globals
/// ===================================================================================
AugSynthPreset gLoadedPreset;

void LoadFactoryPreset(uint8_t idx)
{
    memcpy_P(&gLoadedPreset, &gFactoryPresets[idx], sizeof(AugSynthPreset));
}

void LoadFactoryPresetNameOnly(uint8_t idx)
{
    memcpy_P(gLoadedPreset.mName, gFactoryPresets[idx].mName, sizeof(char[PRESET_NUM_CHARS]));
}

void LoadUserPreset(uint8_t idx)
{
    WaitForEEPROM();

    uintptr_t iptr = idx * sizeof(AugSynthPreset) + EEPROM_PRESET_OFFSET;
    eeprom_read_block(&gLoadedPreset, (void*)iptr, sizeof(AugSynthPreset));
}

void LoadUserPresetNameOnly(uint8_t idx)
{
    WaitForEEPROM();

    uintptr_t iptr = idx * sizeof(AugSynthPreset) + EEPROM_PRESET_OFFSET;
    eeprom_read_block(gLoadedPreset.mName, (void*)iptr, sizeof(char[PRESET_NUM_CHARS]));
}

void SaveUserPreset(uint8_t idx)
{
    WaitForEEPROM();

    // This will take a while so shut everything off.
    SendNoteOffAllCh();

    uintptr_t iptr = idx * sizeof(AugSynthPreset) + EEPROM_PRESET_OFFSET;
    eeprom_write_block(&gLoadedPreset, (void*)iptr, sizeof(AugSynthPreset));
}

bool LoadSlotExists(uint8_t idx)
{
    if(idx < NUM_FACTORY_PRESETS)
    {
        return true;
    }
    
    idx -= NUM_FACTORY_PRESETS;
    return SaveSlotExists(idx);
}

bool SaveSlotExists(uint8_t idx)
{
    uint8_t testName;

    WaitForEEPROM();
    uintptr_t iptr = idx * sizeof(AugSynthPreset) + EEPROM_PRESET_OFFSET;
    eeprom_read_block(&testName, (void*)iptr, 1);

    return testName != 0xFF;
}