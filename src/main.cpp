#include <Arduino.h>
#include <MIDI.h>
#include <NotePressInfo.h>
#include <TimeInfo.h>
#include <Debug.h>
#include <VirtualPinToNote.h>
#include <StableState.h>
#include <Tempo.h>
#include <Arp.h>
#include <Constants.h>

/// ===================================================================================
/// Members
/// ===================================================================================
TimeInfo gTime;
TimeInfo gPrevTime;

Arp gArp;
uint8_t gArpStep = 0;

StableState gVirtualMuxPins[NUM_VIRTUAL_MUX_PIN];
NotePressInfo gNoteStates[NUM_NOTES];

/// ===================================================================================
/// Setup
/// ===================================================================================
MIDI_CREATE_DEFAULT_INSTANCE();

//-- Arduino intrinsic. Prog entry point.
void setup() 
{
	gTime = TimeInfo();
	SetTempo(DEFAULT_TEMPO);

	gArp.mMode = ArpMode::ARP_UP;

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

	for (uint8_t i = 0; i < NUM_NOTES; i++)
	{
		gNoteStates[i] = NotePressInfo();
	}
}





/// ===================================================================================
/// Update
/// ===================================================================================
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
	ReadVirtualPins();
}


//-- Update note states array, send midi commands.
void PlayNotes()
{
	for (int i = 0; i < NUM_NOTES; i++)
	{
		uint8_t vPinIdx = NOTES_VPIN_START + i;
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		
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

void PlayArpUp()
{
    bool note4 = On4Note(gPrevTime.mTimeMs, gTime.mTimeMs);
    bool note8 = On8Note(gPrevTime.mTimeMs, gTime.mTimeMs);

    bool noteOn = note4;
    bool noteOff = !note4 && note8;

    if (!note4 && !note8)
    {
        return;
    }

    uint8_t numPressed = 0;

	for (int i = 0; i < NUM_NOTES; i++)
	{
		uint8_t vPinIdx = NOTES_VPIN_START + i;
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		uint8_t noteNum = VirtualPinToNote(i);
		
		gNoteStates[i].ChangeState(vPinState, gTime.mTimeMs);

		bool pressed = gNoteStates[i].mPressed;

		if (pressed)
		{
            if (numPressed == gArpStep)
            {
                if (noteOn)
                {
					MIDI.sendNoteOn(noteNum, 100, 1); 
                }
                else if (noteOff)
                {
                    MIDI.sendNoteOff(noteNum, 100, 1);
                }
            }
            numPressed++;
		}
	}

    if (noteOff)
    {
        gArpStep++;
    }

	if (gArpStep >= numPressed)
	{
		gArpStep = 0;
	}
}


//-- Arduino intrinsic. Runs in loop.
void loop()
{
	gPrevTime = gTime;
	gTime.PollTime();

	ReadAllPins();

	if (gArp.mMode != ARP_OFF)
	{
		PlayArpUp(); // Temp test
	}
	else
	{
		PlayNotes();
	}

	//char msgBuff[1024];
	//sprintf(msgBuff, "Reg 0x%" PRIx32 " 0x%" PRIx32, lower, upper);
	// for (int i = 0; i < NUM_VIRTUAL_MUX_PIN; i++)
	// {
	// 	Reg
	// }
	//Serial.println(msgBuff);

	// for (uint8_t i = 0; i < NUM_VIRTUAL_MUX_PIN; i++)
	// {
	// 	if(gVirtualMuxPins[i].IsActive())
	// 	{
	// 		Serial.println(gVirtualMuxPins[i].mState);
	// 	}
	// }

	// MIDI.sendNoteOn(80, 100, 1);
	// delay(1000);
	// MIDI.sendNoteOff(80, 100, 1);
	// delay(1000);

	//int timeTaken = millis() - mTime.mTimeMs;
	//Serial.println("timeTaken");
}