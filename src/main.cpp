#include <Arduino.h>
#include <MIDI.h>
#include <NotePressInfo.h>
#include <TimeInfo.h>
#include <Debug.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr int NOTE_PIN_START = 30;
constexpr int NUM_NOTES = 3;
constexpr int NOTE_START = 60;

constexpr unsigned long DEFAULT_TEMPO_INTERVAL = 500; // 120BPM




/// ===================================================================================
/// Members
/// ===================================================================================
TimeInfo mTime;
TimeInfo mPrevTime;

bool mNotePinStates[NUM_NOTES];
NotePressInfo mNoteStates[NUM_NOTES];

unsigned long mTempoInterval = DEFAULT_TEMPO_INTERVAL;





/// ===================================================================================
/// Setup
/// ===================================================================================
MIDI_CREATE_DEFAULT_INSTANCE();

//-- Arduino intrinsic. Prog entry point.
void setup() 
{
	mTime = TimeInfo();
	mTempoInterval = DEFAULT_TEMPO_INTERVAL;

	MIDI.begin(MIDI_CHANNEL_OFF);
	for (int i = 0; i < NUM_NOTES; i++)
	{
		int pinNum = i + NOTE_PIN_START;
		mNotePinStates[i] = false;
		mNoteStates[i] = NotePressInfo();
		pinMode(pinNum, INPUT_PULLUP);
	}
}





/// ===================================================================================
/// Update
/// ===================================================================================

//-- Read all pins
void ReadAllPins()
{
	for (int i = 0; i < NUM_NOTES; i++)
	{
		int pinNum = i + NOTE_PIN_START;
		mNotePinStates[i] = digitalRead(pinNum) == LOW;
	}
}


//-- Update note states array.
void UpdateNoteStates()
{
	int noteNum = 0;

	for (int i = 0; i < NUM_NOTES; i++)
	{
		noteNum = NOTE_START + i;
		bool prevPressed = mNoteStates[i].mPressed;

		mNoteStates[i].ChangeState(mNotePinStates[i], mTime.mTimeMs);

		bool pressed = mNoteStates[i].mPressed;

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
	mPrevTime = mTime;
	mTime.PollTime();

	ReadAllPins();
	UpdateNoteStates();

	//int timeTaken = millis() - mTime.mTimeMs;
	//Serial.println(timeTaken);
}