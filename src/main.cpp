#include <Arduino.h>
#include <MIDI.h>
#include <NotePressInfo.h>
#include <TimeInfo.h>
#include <Debug.h>
#include <VirtualPinToNote.h>

/// ===================================================================================
/// Constants
/// ===================================================================================

constexpr uint8_t PIN_MUX_START = 23;
constexpr uint8_t NUM_MUX_PIN = 11;
constexpr uint8_t PIN_MUX_S0 = 51;
constexpr uint8_t PIN_MUX_S1 = 52;
constexpr uint8_t PIN_MUX_S2 = 53;

constexpr size_t NUM_VIRTUAL_MUX_PIN = NUM_MUX_PIN * 8; // 8-to-1 mux chips

constexpr uint8_t VPIN_PRESS_THRESH = 16;
constexpr uint8_t VPIN_PRESS_THRESH_MAX = 32;

constexpr size_t NOTES_VPIN_START = 0;
constexpr uint8_t NUM_NOTES = 88;
constexpr uint8_t NOTE_START = 10;

constexpr uint16_t DEFAULT_TEMPO_INTERVAL = 500; // 120BPM

// Assert constants make sense
static_assert(NUM_NOTES + NOTES_VPIN_START <= NUM_VIRTUAL_MUX_PIN, 
								"Not enough vpins for notes.");
static_assert((int)NOTE_START + (int)NUM_NOTES < 128, "Note index exceeds midi maximum");



/// ===================================================================================
/// Members
/// ===================================================================================
TimeInfo gTime;
TimeInfo gPrevTime;

uint8_t gVirtualMuxPins[NUM_VIRTUAL_MUX_PIN];
NotePressInfo gNoteStates[NUM_NOTES];

unsigned long gTempoInterval = DEFAULT_TEMPO_INTERVAL;





/// ===================================================================================
/// Setup
/// ===================================================================================
MIDI_CREATE_DEFAULT_INSTANCE();

//-- Arduino intrinsic. Prog entry point.
void setup() 
{
	gTime = TimeInfo();
	gTempoInterval = DEFAULT_TEMPO_INTERVAL;

	MIDI.begin(MIDI_CHANNEL_OFF);

	// Init pins
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

	// Init virtual pins
	for (uint8_t i = 0; i < NUM_VIRTUAL_MUX_PIN; i++)
	{
		gVirtualMuxPins[i] = false;
	}

	for (uint8_t i = 0; i < NUM_NOTES; i++)
	{
		gNoteStates[i] = NotePressInfo();
	}
}





/// ===================================================================================
/// Update
/// ===================================================================================


bool VirtualPinTrue(uint8_t vpinIdx)
{
	return gVirtualMuxPins[vpinIdx] >= 0x80;
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

				while(s2 && digitalRead(PIN_MUX_S2) == LOW)
				{
					//// TEMP HACK
				}

				for (uint8_t i = 0; i < NUM_MUX_PIN; i++)
				{
					uint8_t pinNum = i + PIN_MUX_START;
					uint8_t pinState = digitalRead(pinNum);

					if (pinState == HIGH && gVirtualMuxPins[idx] > 0)
					{
						gVirtualMuxPins[idx]--;
						if (gVirtualMuxPins[idx] == 0x80)
						{
							gVirtualMuxPins[idx] = 0;
						}
					}
					else if(pinState == LOW && gVirtualMuxPins[idx] < VPIN_PRESS_THRESH_MAX + 0x80)
					{
						gVirtualMuxPins[idx]++;
						if (gVirtualMuxPins[idx] == VPIN_PRESS_THRESH)
						{
							gVirtualMuxPins[idx] = VPIN_PRESS_THRESH + 0x80;
						}
					}
					idx++;
				}
			}	
		}
	}
}

//-- Read all pins
void ReadAllPins()
{
	ReadVirtualPins();
}


//-- Update note states array.
void UpdateNoteStates()
{
	for (int i = 0; i < NUM_NOTES; i++)
	{
		uint8_t vPinIdx = NOTES_VPIN_START + i;
		bool vPinState = VirtualPinTrue(vPinIdx);
		
		bool prevPressed = gNoteStates[i].mPressed;
		gNoteStates[i].ChangeState(vPinState, gTime.mTimeMs);

		uint8_t noteNum = VirtualPinToNote(i);

		bool pressed = gNoteStates[i].mPressed;

		if (pressed)
		{
			if (!prevPressed)
			{
				MIDI.sendNoteOn(noteNum, 100, 1);
			}
		}
		else
		{
			if (prevPressed)
			{
				MIDI.sendNoteOff(noteNum, 0, 1);
			}
		}
	}
}


//-- Arduino intrinsic. Runs in loop.
void loop()
{
	gPrevTime = gTime;
	gTime.PollTime();

	ReadAllPins();
	UpdateNoteStates();

	//char msgBuff[1024];
	//sprintf(msgBuff, "Reg 0x%" PRIx32 " 0x%" PRIx32, lower, upper);
	// for (int i = 0; i < NUM_VIRTUAL_MUX_PIN; i++)
	// {
	// 	Reg
	// }
	//Serial.println(msgBuff);

	// for (int i = 0; i < NUM_VIRTUAL_MUX_PIN; i++)
	// {
	// 	if(gVirtualMuxPins[i] > 0)
	// 	{
	// 		Serial.println(i);
	// 	}
	// }

	// MIDI.sendNoteOn(80, 100, 1);
	// delay(1000);
	// MIDI.sendNoteOff(80, 100, 1);
	// delay(1000);

	//int timeTaken = millis() - mTime.mTimeMs;
	//Serial.println("timeTaken");
}