#include <Arduino.h>
#include <AugSynthParams.h>

#ifndef AUG_SYNTH_PRESET_H
#define AUG_SYNTH_PRESET_H
constexpr size_t NUM_FACTORY_PRESETS = 3;
constexpr uint8_t PRESET_NUM_CHARS = 8;
constexpr uintptr_t EEPROM_PRESET_OFFSET = 0;

struct AugSynthPreset
{
    char mName[PRESET_NUM_CHARS];
    int8_t mValues[ASP_NUM_PARAMS];
};


extern AugSynthPreset gLoadedPreset;

void LoadFactoryPreset(uint8_t idx);
void LoadFactoryPresetNameOnly(uint8_t idx);

void LoadUserPreset(uint8_t idx);
void LoadUserPresetNameOnly(uint8_t idx);
void SaveUserPreset(uint8_t idx);

#endif // AUG_SYNTH_PRESET_H