#include <MidiOutput.h>

#ifndef LOC_STRINGS_H
#define LOC_STRINGS_H

/// @brief Convert pedal mode to string (max 10 len)
const char* const PedalModeToChars(PedalMode mode);

/// @brief Convert pedal mode to string (max 2 len)
const char* const PedalModeToCharsShort(PedalMode mode);

/// @brief Convert pedal internal param to string
const char* const PedalInternalToString(PedalInternalParam param);

/// @brief Convert tuning to string(max 7 len)
const char* const TuningToString(uint8_t tuning);

/// @brief Convert delay mode to string (max 7 len)
const char* const DelayModeToString(uint8_t delay);

/// @brief Convert delay mode to string (max 7 len)
const char* const SoundTypeToString(uint8_t st);

/// @brief Convert osc mode to string (max 7 len)
const char* const OscModeToString(uint8_t osc);

/// @brief Convert filter mode to string (max 7 len)
const char* const FilterModeToString(uint8_t filt);

/// @brief Convert numeric paramter to string(max 4 len) 
const char* const AugNumberParamToString(uint8_t param);

#endif // LOC_STRINGS_H