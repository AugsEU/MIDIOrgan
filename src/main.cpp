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

#define PROFILING_ENABLED 0

/// ===================================================================================
/// Members
/// ===================================================================================
uTimeMs gTime;
uTimeMs gPrevTime;

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

	delay(100); // Delay to make sure BP synth is online.
	
	gTime = millis();
}





/// ===================================================================================
/// Update
/// ===================================================================================

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
		PlayNotesDirect(0, NUM_NOTES);
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