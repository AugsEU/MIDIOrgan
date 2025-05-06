#include <Arduino.h>
#include <Constants.h>
#include <StableState.h>
#include <StableAnalog.h>

#ifndef USER_CONTROLS_H
#define USER_CONTROLS_H

constexpr uint8_t ANALOG_READ_RESOLUTION_BITS = 10;
constexpr uint16_t ANALOG_MAX_VALUE = 1 << ANALOG_READ_RESOLUTION_BITS;

// Global digital pins
extern StableState gdpArpSelectUpper;
extern StableState gdpArpSelectLower;
extern StableState gdpArpHold;
extern StableState gdpArpUp;
extern StableState gdpArpDown;
extern StableState gdpArpSpec;
extern StableState gdpArpFast;
extern StableState gdpArpSlow;
extern StableState gdpMetronome;
extern StableState gdpLoop1;
extern StableState gdpLoop2;
extern StableState gdpLoop3;
extern StableState gdpLoop4;

// Global analog pins
extern uint16_t gapArpGate;
extern uint16_t gapMidiChUpper;
extern uint16_t gapMidiChLower;
extern uint16_t gapOctaveUpper;
extern uint16_t gapOctaveLower;
extern uint16_t gapTempo;
extern uint16_t gapKnob6;
extern uint16_t gapKnob7;

// Virtual multiplexer pins
extern StableState gVirtualMuxPins[NUM_VIRTUAL_MUX_PIN];

template<typename T, uint8_t divisions, T minValue>
struct AnalogSelector
{
    T mValue;

    AnalogSelector()
    {
        mValue = 0;
    }

    void ForceSelection(uint16_t analog)
    {
        uint16_t region = (analog * divisions) / ANALOG_MAX_VALUE;
        mValue = (T)region + minValue;
    }

    T CalcNextSelection(uint16_t analog)
    {
        uint16_t region = (analog * divisions) / ANALOG_MAX_VALUE;
        uint16_t deadzoneSize = ANALOG_MAX_VALUE / (divisions * 4);
    
        if ((analog + deadzoneSize) * divisions > (region + 1) * ANALOG_MAX_VALUE)
        {
            // In upper deadzone do not update
            return mValue;
        }
    
        if ((analog < deadzoneSize) || (analog - deadzoneSize) * divisions < region * ANALOG_MAX_VALUE)
        {
            // In lower deadzone do not update
            return mValue;
        }
    
        return (T)region + minValue;
    }

    constexpr static T GetMinValue()
    {
        return minValue;
    }
};

// Public funcs
void SetupPins();
void ReadAllPins();
uint16_t GetPedalStable();

// Debug
void DebugDigitalPins();
void DebugAnalogPins();

#endif // USER_CONTROLS_H