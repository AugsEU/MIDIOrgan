#include <Arduino.h>
#include <Util/Constants.h>
#include <Input/StableState.h>
#include <Input/StableAnalog.h>
#include <Input/RotaryEncoder.h>

#ifndef USER_CONTROLS_H
#define USER_CONTROLS_H

constexpr uint8_t ANALOG_READ_RESOLUTION_BITS = 10;
constexpr uint16_t ANALOG_MAX_VALUE = 1 << ANALOG_READ_RESOLUTION_BITS;
constexpr uint8_t NUM_ROTARY_ENCODERS = 7;

// Global digital pins
extern StableState<16> gdpArpSelectUpper;
extern StableState<16> gdpArpSelectLower;
extern StableState<16> gdpArpHold;
extern StableState<16> gdpArpUp;
extern StableState<16> gdpArpDown;
extern StableState<16> gdpArpSpec;
extern StableState<16> gdpArpFast;
extern StableState<16> gdpArpSlow;
extern StableState<16> gdpMetronome;
extern StableState<16> gdpLoop1;
extern StableState<16> gdpLoop2;
extern StableState<16> gdpLoop3;
extern StableState<16> gdpLoop4;

// Global analog pins
extern uint16_t gapArpGate;
extern uint16_t gapMidiChUpper;
extern uint16_t gapMidiChLower;
extern uint16_t gapOctaveUpper;
extern uint16_t gapOctaveLower;
extern uint16_t gapTempo;
extern uint16_t gapPedalMode;
extern uint16_t gapPedalSelect;

// Virtual multiplexer pins
extern StableState<16> gVirtualMuxPins[NUM_VIRTUAL_MUX_PIN];
extern RotaryEncoder gRotaryEncoders[NUM_ROTARY_ENCODERS];

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

        constexpr uint16_t deadzoneSize = ANALOG_MAX_VALUE / (divisions * 4);
        uint16_t prevRegion = (uint16_t)(mValue - minValue);
        if (region == prevRegion + 1) // Going to region 1 above
        {
            if(analog * divisions <= prevRegion * ANALOG_MAX_VALUE + deadzoneSize * divisions)
            {
                return mValue;
            }
        }
        else if(region + 1 == prevRegion) // Going to region 1 below
        {
            if((analog + deadzoneSize) * divisions >= prevRegion * ANALOG_MAX_VALUE)
            {
                return mValue;
            }
        }
    
        // if ((analog + deadzoneSize) * divisions > (mValue + 1) * ANALOG_MAX_VALUE)
        // {
        //     // In upper deadzone do not update
        //     return mValue;
        // }
    
        // if ((analog < deadzoneSize) || (analog - deadzoneSize) * divisions < mValue * ANALOG_MAX_VALUE)
        // {
        //     // In lower deadzone do not update
        //     return mValue;
        // }
    
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
void PollRotaryEncoders();
uint32_t GetPedalStable();

// Debug
void DebugDigitalPins();
void DebugAnalogPins();

#endif // USER_CONTROLS_H