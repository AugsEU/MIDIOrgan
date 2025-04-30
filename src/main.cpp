#include <Arduino.h>
#include <NotePressInfo.h>
#include <TimeInfo.h>
#include <Debug.h>
#include <VirtualPinToNote.h>
#include <StableState.h>
#include <Tempo.h>
#include <Arp.h>
#include <Constants.h>
#include <MidiOutput.h>
#include <ScreenDisplay.h>
#include <UserControls.h>

#define PROFILING_ENABLED 1

/// ===================================================================================
/// Members
/// ===================================================================================
uTimeMs gTime;
uTimeMs gPrevTime;

NotePressInfo gNoteStates[NUM_NOTES];

#if PROFILING_ENABLED
constexpr size_t LOOP_PROFILE_LIMIT = 10000;
uTimeMs gFirstLoopTime = 0;
size_t gLoopCount = 0;
#endif // PROFILING_ENABLED


/// ===================================================================================
/// Setup
/// ===================================================================================

/// @brief Arduino intrinsic. Prog entry point.
void setup() 
{
	LcdInit();

	// Init pins
	SetupPins();
	ReadAllPins();

	for (uint8_t i = 0; i < NUM_NOTES; i++)
	{
		gNoteStates[i] = NotePressInfo();
	}
	
	MidiOutputSetup();
	
	gTime = millis();
}





/// ===================================================================================
/// Update
/// ===================================================================================

/// @brief Play keys like a regular piano.
void PlayNotesDirect()
{
	for (int i = 0; i < NUM_NOTES; i++)
	{
		uint8_t vPinIdx = NOTES_VPIN_START + i;
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		
		bool prevPressed = gNoteStates[i].mPressed;
		gNoteStates[i].ChangeState(vPinState, gTime);

		uint8_t keyNum = VirtualPinToKeyNum(i);
		bool pressed = gNoteStates[i].mPressed;

		if (pressed)
		{
			if (!prevPressed)
			{
				SendNoteOn(keyNum);
			}
		}
		else
		{
			if (prevPressed)
			{
				SendNoteOff(keyNum);
			}
		}
	}
}

//-- Arduino intrinsic. Runs in loop.
void loop()
{
	gPrevTime = gTime;
	gTime = millis();

	ReadAllPins();
	ReadArpMode();
	UpdateTempo();
	UpdateMidiOutput();
	UpdateScreen();

	if (ArpEnabled())
	{
		PlayArp();
	}
	else
	{
		PlayNotesDirect();
	}

	// VPIN TEST
	// for (uint8_t i = 0; i < NUM_VIRTUAL_MUX_PIN; i++)
	// {
	// 	if(gVirtualMuxPins[i].IsActive())
	// 	{
	// 		Serial.println(gVirtualMuxPins[i].mState);
	// 	}
	// }

	// SWITCH TEST
	// uint8_t midiChTest = 0;
	// GetAnalogSelectionValue(&midiChTest, gapMidiChUpper, 17);
	// Serial.println(midiChTest);
	// DebugDigitalPins();
	//DebugAnalogPins();

#if PROFILING_ENABLED
	if (gLoopCount == 0)
	{
		gFirstLoopTime = gTime;
	}
	gLoopCount++;
	
	if (gLoopCount == LOOP_PROFILE_LIMIT)
	{
		double timeTaken = millis() - gFirstLoopTime;
		timeTaken /= (double)LOOP_PROFILE_LIMIT;
		Serial.println(timeTaken);
	}
#endif // PROFILING_ENABLED
}