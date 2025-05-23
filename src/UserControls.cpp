#include <Input/StableState.h>
#include <UserControls.h>
#include <Input/StableAnalog.h>
#include "wiring_private.h"
#include "pins_arduino.h"

#define READ_SECTIONS 0
#define DIRECT_PORT_READ 1

constexpr uint16_t PEDAL_MIN = 20;
constexpr uint16_t PEDAL_MAX = 915;
constexpr int ANALOG_READ_CYCLES = 16;
constexpr uint8_t ROTARY_IDXS_L[] = { 10, 8, 11, 14, 12, 9, 13};
constexpr uint8_t ROTARY_IDXS_R[] = { 3,  4,  6,  0,  5, 1,  2};

StableState<16> gdpArpSelectUpper;
StableState<16> gdpArpSelectLower;
StableState<16> gdpArpHold;
StableState<16> gdpArpUp;
StableState<16> gdpArpDown;
StableState<16> gdpArpSpec;
StableState<16> gdpArpFast;
StableState<16> gdpArpSlow;
StableState<16> gdpMetronome;
StableState<16> gdpLoop1;
StableState<16> gdpLoop2;
StableState<16> gdpLoop3;
StableState<16> gdpLoop4;

uint16_t gapArpGate;
uint16_t gapMidiChUpper;
uint16_t gapMidiChLower;
uint16_t gapOctaveUpper;
uint16_t gapOctaveLower;
uint16_t gapTempo;
uint16_t gapPedalMode;
uint16_t gapPedalSelect;
uint16_t gapPedalValue;
StableAnalog gStablePedal;
uint32_t gPedalValueCache = 0;

StableState<5> gVirtualMuxPins[NUM_VIRTUAL_MUX_PIN];
RotaryEncoder gRotaryEncoders[NUM_ROTARY_ENCODERS];
StableState<4> gRotaryEncoderMuxPins[8*2];

uint8_t gAnalogReadSection = 0;
uint8_t gAnalogReadingPin = 0xFF;

void SetupPins()
{
    pinMode(PIN_MUX_S0, OUTPUT);
	digitalWrite(PIN_MUX_S0, LOW);
	pinMode(PIN_MUX_S1, OUTPUT);
	digitalWrite(PIN_MUX_S1, LOW);
	pinMode(PIN_MUX_S2, OUTPUT);
	digitalWrite(PIN_MUX_S2, LOW);

	for(uint8_t i = 0; i < NUM_MUX_PIN; i++)
	{
		uint8_t pinNum = i + PIN_MUX_START;
		pinMode(pinNum, INPUT_PULLUP);
	}
	pinMode(PIN_MUX_RE_LEFT, INPUT_PULLUP);
	pinMode(PIN_MUX_RE_RIGHT, INPUT_PULLUP);

	pinMode(PIN_ARP_SELECT_UPPER, INPUT_PULLUP);
	pinMode(PIN_ARP_SELECT_LOWER, INPUT_PULLUP);
	pinMode(PIN_ARP_HOLD, INPUT_PULLUP);
	pinMode(PIN_ARP_UP, INPUT_PULLUP);
	pinMode(PIN_ARP_DOWN, INPUT_PULLUP);
	pinMode(PIN_ARP_SPEC, INPUT_PULLUP);
	pinMode(PIN_ARP_FAST, INPUT_PULLUP);
	pinMode(PIN_ARP_SLOW, INPUT_PULLUP);
	pinMode(PIN_METRONOME, INPUT_PULLUP);
	pinMode(PIN_LOOP1, INPUT_PULLUP);
	pinMode(PIN_LOOP2, INPUT_PULLUP);
	pinMode(PIN_LOOP3, INPUT_PULLUP);
	pinMode(PIN_LOOP4, INPUT_PULLUP);
}

void BeginAnalogRead(uint8_t pin)
{
	gAnalogReadingPin = pin;
	if (pin >= 54)
	{
		pin -= 54; // Allow for channel or pin numbers
	}

	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
	ADMUX = (1 << 6) | (pin & 0x07);

	// Start the conversion
	sbi(ADCSRA, ADSC);
}

bool CurrentlyReadingADC()
{
	return gAnalogReadingPin != 0xFF;
}

bool AnalogReadEndReady()
{
	return !(bit_is_set(ADCSRA, ADSC));
}

uint16_t EndAnalogRead()
{
	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	gAnalogReadingPin = 0xFF;

	// ADC macro takes care of reading ADC register.
	// avr-gcc implements the proper reading order: ADCL is read first.
	return ADC;
}

void BeginAnalogReadForMux()
{
	switch (gAnalogReadSection)
	{
	case 0:
		BeginAnalogRead(PINA_ARP_GATE);
		break;
	case 1:
		BeginAnalogRead(PINA_MIDI_CH_UPPER);
		break;
	case 2:
		BeginAnalogRead(PINA_MIDI_CH_LOWER);
		break;
	case 3:
		BeginAnalogRead(PINA_OCTAVE_UPPER);
		break;
	case 4:
		BeginAnalogRead(PINA_OCTAVE_LOWER);
		break;
	case 5:
		BeginAnalogRead(PINA_TEMPO);
		break;
	case 6:
		BeginAnalogRead(PINA_KNOB6);
		break;
	case 7:
		BeginAnalogRead(PINA_KNOB7);
		break;
	case 8:
		BeginAnalogRead(PINA_PEDAL);
		break;
	default:
		Serial.println("ANALOG READ ERROR");
		break;
	}
}

void EndAnalogReadForMux()
{
	constexpr uint32_t PEDAL_RANGE = PEDAL_MAX - PEDAL_MIN;

	switch (gAnalogReadSection)
	{
	case 0:
		gapArpGate = EndAnalogRead();
		break;
	case 1:
		gapMidiChUpper = EndAnalogRead();
		break;
	case 2:
		gapMidiChLower = EndAnalogRead();
		break;
	case 3:
		gapOctaveUpper = EndAnalogRead();
		break;
	case 4:
		gapOctaveLower = EndAnalogRead();
		break;
	case 5:
		gapTempo = EndAnalogRead();
		break;
	case 6:
		gapPedalMode = EndAnalogRead();
		break;
	case 7:
		gapPedalSelect = EndAnalogRead();
		break;
	case 8:
		gStablePedal.ConsumeInput(EndAnalogRead());
		{
			gPedalValueCache = gStablePedal.GetStableValue(); // need 32 to avoid overflow
			gPedalValueCache = min(gPedalValueCache, PEDAL_MAX);
			gPedalValueCache = max(gPedalValueCache, PEDAL_MIN); // clamp

			gPedalValueCache = PEDAL_MAX - gPedalValueCache; //Invert to because of wiring
			gPedalValueCache <<= ANALOG_READ_RESOLUTION_BITS;
			gPedalValueCache /= PEDAL_RANGE;
		}
		break;
	default:
		Serial.println("ANALOG READ ERROR");
		break;
	}

	gAnalogReadSection++;
	if(gAnalogReadSection > 8)
	{
		gAnalogReadSection = 0;
	}
}

void UpdateRotaryEncoders()
{
	for(uint8_t i = 0; i < NUM_ROTARY_ENCODERS; i++)
	{
		bool left = gRotaryEncoderMuxPins[ROTARY_IDXS_L[i]].IsActive();
		bool right = gRotaryEncoderMuxPins[ROTARY_IDXS_R[i]].IsActive();
		gRotaryEncoders[i].UpdateDial(left, right);
	}
}

void PollRotaryEncoders()
{
	uint8_t idx = 0;
	for (uint8_t s2 = 0; s2 <= 1; s2++)
	{
		digitalWrite(PIN_MUX_S2, s2 ? LOW : HIGH);
		for (uint8_t s1 = 0; s1 <= 1; s1++)
		{
			digitalWrite(PIN_MUX_S1, s1 ? LOW : HIGH);
			for (uint8_t s0 = 0; s0 <= 1; s0++)
			{
				digitalWrite(PIN_MUX_S0, s0 ? LOW : HIGH);
				delayMicroseconds(10);

				idx = 4*s2 + 2*s1 + s0;
				gRotaryEncoderMuxPins[idx].UpdateState(PORT_DPIN_34 == 0);
				gRotaryEncoderMuxPins[idx+8].UpdateState(PORT_DPIN_35 == 0);
			}
		}
	}

	UpdateRotaryEncoders();
}

void ClearRotaryEncoderDeltas()
{
	for(uint8_t i = 0; i < NUM_ROTARY_ENCODERS; i++)
	{
		gRotaryEncoders[i].mValue = 0;
		gRotaryEncoders[i].mLatch = 0;
	}
}

void ReadVirtualPins()
{
	uint8_t idx = 0;
	for (uint8_t s2 = 0; s2 <= 1; s2++)
	{
		digitalWrite(PIN_MUX_S2, s2 ? LOW : HIGH);
		for (uint8_t s1 = 0; s1 <= 1; s1++)
		{
			digitalWrite(PIN_MUX_S1, s1 ? LOW : HIGH);
			for (uint8_t s0 = 0; s0 <= 1; s0++)
			{
				digitalWrite(PIN_MUX_S0, s0 ? LOW : HIGH);

				// Delay needed for mux chips to change state. (6 is min, 15 to be safe since ADC takes a while anyway)
				if(!CurrentlyReadingADC()) // While we are waiting we can also set the ADC going..
				{
					BeginAnalogReadForMux();
				} 
				delayMicroseconds(20);

				idx = 4*s2 + 2*s1 + s0;
#if DIRECT_PORT_READ && 0
				gVirtualMuxPins[idx+8*0] .UpdateState(PORT_DPIN_22 == 0);
				gVirtualMuxPins[idx+8*1] .UpdateState(PORT_DPIN_23 == 0);
				gVirtualMuxPins[idx+8*2] .UpdateState(PORT_DPIN_24 == 0);
				gVirtualMuxPins[idx+8*3] .UpdateState(PORT_DPIN_25 == 0);
				gVirtualMuxPins[idx+8*4] .UpdateState(PORT_DPIN_26 == 0);
				gVirtualMuxPins[idx+8*5] .UpdateState(PORT_DPIN_27 == 0);
				gVirtualMuxPins[idx+8*6] .UpdateState(PORT_DPIN_28 == 0);
				gVirtualMuxPins[idx+8*7] .UpdateState(PORT_DPIN_29 == 0);
				gVirtualMuxPins[idx+8*8] .UpdateState(PORT_DPIN_30 == 0);
				gVirtualMuxPins[idx+8*9] .UpdateState(PORT_DPIN_31 == 0);
				gVirtualMuxPins[idx+8*10].UpdateState(PORT_DPIN_32 == 0);
				gVirtualMuxPins[idx+8*11].UpdateState(PORT_DPIN_33 == 0);

				gRotaryEncoderMuxPins[idx].UpdateState(PORT_DPIN_34 == 0);
				gRotaryEncoderMuxPins[idx+8].UpdateState(PORT_DPIN_35 == 0);
#else
				for(uint8_t i = 0; i < NUM_MUX_PIN; i++)
				{
					gVirtualMuxPins[idx+8*i].UpdateState(digitalRead(PIN_MUX_START + i) == LOW);
				}

				gRotaryEncoderMuxPins[idx].UpdateState(digitalRead(PIN_MUX_RE_LEFT) == LOW);
				gRotaryEncoderMuxPins[idx+8].UpdateState(digitalRead(PIN_MUX_RE_RIGHT) == LOW);
#endif
				if(AnalogReadEndReady())
				{
					EndAnalogReadForMux();
				}
			}	
		}
	}

	if(CurrentlyReadingADC())
	{
		EndAnalogReadForMux();
	}

	UpdateRotaryEncoders();
}

//-- Read all pins
void ReadAllPins()
{
	// Read these every frame to minimise input delay for playing keys.
	ReadVirtualPins();

#if DIRECT_PORT_READ
	gdpArpSelectUpper.UpdateState(PORT_ARP_SELECT_UPPER != 0);
	gdpArpSelectLower.UpdateState(PORT_ARP_SELECT_LOWER != 0);
	gdpArpHold.UpdateState(PORT_ARP_HOLD != 0);
	gdpArpUp.UpdateState(PORT_ARP_UP == 0);
	gdpArpDown.UpdateState(PORT_ARP_DOWN == 0);
	gdpArpSpec.UpdateState(PORT_ARP_SPEC == 0);
	gdpArpFast.UpdateState(PORT_ARP_FAST == 0);
	gdpArpSlow.UpdateState(PORT_ARP_SLOW == 0);
	gdpMetronome.UpdateState(PORT_METRONOME != 0);
	gdpLoop1.UpdateState(PORT_LOOP1 != 0);
	gdpLoop2.UpdateState(PORT_LOOP2 != 0);
	gdpLoop3.UpdateState(PORT_LOOP3 != 0);
	gdpLoop4.UpdateState(PORT_LOOP4 != 0);
#else
	gdpArpSelectUpper.UpdateState(digitalRead(PIN_ARP_SELECT_UPPER));
	gdpArpSelectLower.UpdateState(digitalRead(PIN_ARP_SELECT_LOWER));
	gdpArpHold.UpdateState(digitalRead(PIN_ARP_HOLD));
	gdpArpUp.UpdateState(!digitalRead(PIN_ARP_UP));
	gdpArpDown.UpdateState(!digitalRead(PIN_ARP_DOWN));
	gdpArpSpec.UpdateState(!digitalRead(PIN_ARP_SPEC));
	gdpArpFast.UpdateState(!digitalRead(PIN_ARP_FAST));
	gdpArpSlow.UpdateState(!digitalRead(PIN_ARP_SLOW));
	gdpMetronome.UpdateState(digitalRead(PIN_METRONOME));
	gdpLoop1.UpdateState(digitalRead(PIN_LOOP1));
	gdpLoop2.UpdateState(digitalRead(PIN_LOOP2));
	gdpLoop3.UpdateState(digitalRead(PIN_LOOP3));
	gdpLoop4.UpdateState(digitalRead(PIN_LOOP4));
#endif

	gapMidiChUpper = analogRead(PINA_MIDI_CH_UPPER);
	gapMidiChLower = analogRead(PINA_MIDI_CH_LOWER);
}

uint32_t GetPedalStable()
{
	return gPedalValueCache;
}

// Debug
void DebugDigitalPins()
{
    char msgBuff[32];
	int len = 0;
	msgBuff[len++] = gdpArpSelectUpper.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpArpSelectLower.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpArpHold.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpArpUp.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpArpDown.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpArpSpec.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpArpFast.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpArpSlow.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpMetronome.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpLoop1.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpLoop2.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpLoop3.IsActive() ? '1' : '0';
	msgBuff[len++] = gdpLoop4.IsActive() ? '1' : '0';
	msgBuff[len++] = '\0';
	Serial.println(msgBuff);
}

void DebugAnalogPins()
{
	char msgBuff[512];
	sprintf(msgBuff, "%u %u %u %u %u %u %u %u",
		gapArpGate,
		gapMidiChUpper,
		gapMidiChLower,
		gapOctaveUpper,
		gapOctaveLower,
		gapTempo,
		gapPedalMode,
		gapPedalSelect
	);

	Serial.println(msgBuff);
}