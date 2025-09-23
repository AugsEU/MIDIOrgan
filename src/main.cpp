#include <Arduino.h>
#include <Input/NotePressInfo.h>
#include <Util/TimeInfo.h>
#include <Util/Debug.h>
#include <Util/VirtualPinToNote.h>
#include <Input/StableState.h>
#include <Tempo.h>
#include <Arp.h>
#include <Util/Constants.h>
#include <MidiOutput.h>
#include <ScreenDisplay.h>
#include <UserControls.h>
#include <AugSynth.h>
#include <Looper.h>

#define VPIN_TEST 0
#define DPIN_TEST 0
#define APIN_TEST 0
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

void TimerOverflowFixup();

/// ===================================================================================
/// Setup
/// ===================================================================================

/// @brief Arduino intrinsic. Prog entry point.
void setup() 
{
	LcdInit();
#if LOOPER
	InitLooper();
#endif // LOOPER
	InitAugSynth();

	// Init pins
	SetupPins();
	ReadAllPins();

	for (uint8_t i = 0; i < NUM_NOTES; i++)
	{
		gNoteStates[i] = NotePressInfo();
	}
	
	MidiOutputSetup();

	delay(100); // Delay to make everything is online.
	
	gTime = millis();
	ClearRotaryEncoderDeltas();
}





/// ===================================================================================
/// Update
/// ===================================================================================

//-- Arduino intrinsic. Runs in loop.
void loop()
{
	gPrevTime = gTime;
	gTime = millis();
	if(gPrevTime > gTime)
	{
		TimerOverflowFixup();
	}
    
	ReadAllPins();
	ReadArpMode();
	UpdateTempo();
#if LOOPER
	UpdateLooper();
#endif // LOOPER
	PollRotaryEncoders();
	UpdateMidiOutput();
	PollRotaryEncoders();
	UpdateScreen();
	PollRotaryEncoders();
	UpdateAugSynth();
	PollRotaryEncoders();

	if (ArpEnabled())
	{
		PlayArp();
	}
	else
	{
		PlayNotesDirect(0, NUM_NOTES);
	}

	PollRotaryEncoders();

#if VPIN_TEST
	for (uint8_t i = 0; i < NUM_VIRTUAL_MUX_PIN; i++)
	{
		if(gVirtualMuxPins[i].IsActive())
		{
			Serial.println(i);
		}
	}
#endif // VPIN_TEST

#if DPIN_TEST
	DebugDigitalPins();
#endif // DPIN_TEST

#if APIN_TEST
	DebugAnalogPins();
#endif // APIN_TEST

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

void TimerOverflowFixup()
{
	for (uint8_t i = 0; i < NUM_NOTES; i++)
	{
		gNoteStates[i] = NotePressInfo();
	}
	ClearRotaryEncoderDeltas();
	ArpTimerOverflowFixup();
	SendNoteOffAllCh();
#if LOOPER
	EraseAllLoops();
#endif //LOOPER
}