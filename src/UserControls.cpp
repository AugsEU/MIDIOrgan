#include <StableState.h>
#include <UserControls.h>

#define READ_SECTIONS 0

StableState gdpArpSelectUpper;
StableState gdpArpSelectLower;
StableState gdpArpHold;
StableState gdpArpUp;
StableState gdpArpDown;
StableState gdpArpSpec;
StableState gdpArpFast;
StableState gdpArpSlow;
StableState gdpMetronome;
StableState gdpLoop1;
StableState gdpLoop2;
StableState gdpLoop3;
StableState gdpLoop4;

uint16_t gapArpGate;
uint16_t gapMidiChUpper;
uint16_t gapMidiChLower;
uint16_t gapOctaveUpper;
uint16_t gapOctaveLower;
uint16_t gapTempo;
uint16_t gapKnob6;
uint16_t gapKnob7;

StableState gVirtualMuxPins[NUM_VIRTUAL_MUX_PIN];

#if READ_SECTIONS
// We don't read everything every frame.
uint8_t gReadSection = 0;
#endif // READ_SECTIONS

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

void ReadVirtualPins()
{
	uint8_t idx = 0;
	for (uint8_t s0 = 0; s0 <= 1; s0++)
	{
		digitalWrite(PIN_MUX_S0, s0 ? HIGH : LOW);
		for (uint8_t s1 = 0; s1 <= 1; s1++)
		{
			digitalWrite(PIN_MUX_S1, s1 ? HIGH : LOW);
			for (uint8_t s2 = 0; s2 <= 1; s2++)
			{
				digitalWrite(PIN_MUX_S2, s2 ? HIGH : LOW);

				// Delay needed for mux chips to change state. @TODO Could this be lower?
				delayMicroseconds(5);

				for (uint8_t i = 0; i < NUM_MUX_PIN; i++)
				{
					uint8_t pinNum = i + PIN_MUX_START;
					uint8_t pinState = digitalRead(pinNum);
					gVirtualMuxPins[idx].UpdateState(pinState == LOW);
					idx++;
				}
			}	
		}
	}
}

//-- Read all pins
void ReadAllPins()
{
	// Read these every frame to minimise input delay for playing keys.
	ReadVirtualPins();

#if READ_SECTIONS // Optimisation to only read some pins every update.
switch (gReadSection)
	{
	case 0:
		gdpArpSelectUpper.UpdateState(digitalRead(PIN_ARP_SELECT_UPPER));
		gdpArpSelectLower.UpdateState(digitalRead(PIN_ARP_SELECT_LOWER));
		gdpArpHold.UpdateState(digitalRead(PIN_ARP_HOLD));
		gdpArpUp.UpdateState(!digitalRead(PIN_ARP_UP));
		gdpArpDown.UpdateState(!digitalRead(PIN_ARP_DOWN));
		gdpArpSpec.UpdateState(!digitalRead(PIN_ARP_SPEC));
		gdpArpFast.UpdateState(!digitalRead(PIN_ARP_FAST));

		gapArpGate = analogRead(PINA_ARP_GATE);
		gapMidiChUpper = analogRead(PINA_MIDI_CH_UPPER);

		gReadSection++;
		break;
	case 1:
		gdpArpSlow.UpdateState(!digitalRead(PIN_ARP_SLOW));
		gdpMetronome.UpdateState(digitalRead(PIN_METRONOME));
		gdpLoop1.UpdateState(digitalRead(PIN_LOOP1));
		gdpLoop2.UpdateState(digitalRead(PIN_LOOP2));
		gdpLoop3.UpdateState(digitalRead(PIN_LOOP3));
		gdpLoop4.UpdateState(digitalRead(PIN_LOOP4));

		gapMidiChLower = analogRead(PINA_MIDI_CH_LOWER);
		gapOctaveUpper = analogRead(PINA_OCTAVE_UPPER);

		gReadSection++;
		break;
	case 2:
		gapOctaveLower = analogRead(PINA_OCTAVE_LOWER);
		gapTempo = analogRead(PINA_TEMPO);
		gapKnob6 = analogRead(PINA_KNOB6);
		gapKnob7 = analogRead(PINA_KNOB7);

		gReadSection = 0;
		break;
	
	default:
		break;
	}
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

	gapMidiChLower = analogRead(PINA_MIDI_CH_LOWER);
	gapOctaveUpper = analogRead(PINA_OCTAVE_UPPER);
	gapOctaveLower = analogRead(PINA_OCTAVE_LOWER);
	gapArpGate = analogRead(PINA_ARP_GATE);
	gapMidiChUpper = analogRead(PINA_MIDI_CH_UPPER);
	gapTempo = analogRead(PINA_TEMPO);
	gapKnob6 = analogRead(PINA_KNOB6);
	gapKnob7 = analogRead(PINA_KNOB7);
#endif
}



/// @brief For an analog input, get one of N selection values.
/// @param pValue Output value
/// @param analog Analog value
/// @param divisions Number of values we can select from.
void GetAnalogSelectionValue(uint8_t* pValue, uint16_t analog, uint16_t divisions)
{
	uint16_t region = (analog * divisions) / ANALOG_MAX_VALUE;
	uint16_t deadzoneSize = ANALOG_MAX_VALUE / (divisions * 4);

	if ((analog + deadzoneSize) * divisions > (region + 1) * ANALOG_MAX_VALUE)
	{
		// In upper deadzone do not update
		return;
	}

	if ((analog < deadzoneSize) || (analog - deadzoneSize) * divisions < region * ANALOG_MAX_VALUE)
	{
		// In lower deadzone do not update
		return;
	}

	// Set to region
	*pValue = (uint8_t)region;
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
		gapKnob6,
		gapKnob7
	);

	Serial.println(msgBuff);
}