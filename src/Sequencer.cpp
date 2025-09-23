/// ===================================================================================
/// Include
/// ===================================================================================

#include <Util/TimeInfo.h>
#include <UserControls.h>
#include <Tempo.h>
#include <Util/Globals.h>
#include <Util/Debug.h>
#include <MidiOutput.h>
#include <Util/AugMath.h>
#include <Util/Utils.h>
#include <Input/DigitalButton.h>
#include <Sequencer.h>
#include <ScreenDisplay.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr uint8_t SEQ_PAGE_SELECT_DIAL_IDX = 0;
constexpr uint8_t SEQ_TOP_LEFT_DIAL = 1;
constexpr uint8_t SEQ_TOP_RIGHT_DIAL = 2;
constexpr uint8_t SEQ_BOT_LEFT_DIAL = 3;
constexpr uint8_t SEQ_BOT_RIGHT_DIAL = 4;
constexpr uint8_t SEQ_VPIN_SEQUENCER_BTN = 5;

/// ===================================================================================
/// Globals
/// ===================================================================================
SequencerTrack gTracks[NUM_SEQ_TRACKS];
int8_t gSeqSelectTrack = 0;
int8_t gSeqSelectStep = 0;
DigitalButton gSeqEditBtn;

/// ===================================================================================
/// SequencerStep
/// ===================================================================================
SequencerStep::SequencerStep()
{
	memset(mNotes, 0, STEP_POLYPHONY);
	mVelocity = 0;
	mLength = 0;
}

/// ===================================================================================
/// SequencerTrack
/// ===================================================================================

/// @brief Init sequencer track.
SequencerTrack::SequencerTrack()
{
	mPlaying = false;
	mMidiCh = 0;
	mSubDiv = 0;
	mNumSteps = 0;

	mPlayHead = 0;
	EraseAllSteps();
}

/// @brief Erase all sequencer steps
void SequencerTrack::EraseAllSteps()
{
	for(uint8_t i = 0; i < TRACK_STEP_MAX; i++)
	{
		mSteps[i] = SequencerStep();
	}
}

/// @brief Write a duration(4-bit) to buffer. 
void SequencerTrack::WritePlayNoteBuf(uint8_t note, uint8_t duration)
{
	uint8_t idx = note / 2;
	if(note % 2) // low bit
	{
		mPlayingNotes[idx] = duration & 0x0F;
	}
	else // high bit
	{
		mPlayingNotes[idx] = (duration & 0x0F) << 4;
	}
}

/// @brief Read a duration(4-bit) from the buffer
uint8_t SequencerTrack::ReadPlayNoteBuf(uint8_t note)
{
	uint8_t idx = note / 2;
	
	uint8_t value;
	if(note % 2) // low bit
	{
		value = mPlayingNotes[idx] & 0x0F;
	}
	else // high bit
	{
		value = (mPlayingNotes[idx] & 0xF0) >> 4;
	}
	
	return value;
}

/// @brief Play notes on and off.
void SequencerTrack::UpdateTrack()
{
	if(On4Note(mSubDiv)) // Step forward.
	{
		if(mPlaying) // Play notes and move head forward.
		{
			PlayStep(&mSteps[mPlayHead]);

			mPlayHead += 1;
			if(mPlayHead >= mNumSteps)
			{
				// Wrap around.
				mPlayHead = 0;
			}
		}

		// Look for notes to turn off. We always do this so notes aren't left hanging.
		for(uint8_t note = 0; note < 128; note++)
		{
			uint8_t duration = ReadPlayNoteBuf(note);
			if(duration > 0)
			{
				duration -= 1;
				if(duration == 0)
				{
					SendNoteOffMidi(note, mMidiCh);
				}

				WritePlayNoteBuf(note, duration);
			}
		}
	}
}


/// @brief Play a note and register when to turn it off.
void SequencerTrack::PlayStep(SequencerStep* step)
{
	for(uint8_t ni = 0; ni < STEP_POLYPHONY; ni++)
	{
		uint8_t note = step->mNotes[ni];
		if(note == 0)
			continue;
		
		uint8_t noteDuration = ReadPlayNoteBuf(note);// if note is still playing we cancel it first.
		if(noteDuration > 0)
		{
			SendNoteOffMidi(note, mMidiCh);
		}

		// Play note
		SendNoteOnMidi(note, step->mVelocity, mMidiCh);

		// Write to buffer to remember when to turn it off.
		WritePlayNoteBuf(note, step->mLength);
	}
	
}

/// @brief Set the track to be playing or not.
void SequencerTrack::SetPlaying(bool playing)
{
	if(playing != mPlaying)
	{
		mPlayHead = 0;// Reset playhead when stopping or starting.
	}
	mPlaying = playing;
}

/// ===================================================================================
/// Static functions
/// ===================================================================================

/// @brief Init all 4 sequencer tracks.
void InitSequencer()
{
	for(uint8_t i = 0; i < NUM_SEQ_TRACKS; i++)
	{
		gTracks[i] = SequencerTrack();
	}
}

/// @brief Update all 4 sequencer tracks.
void UpdateSequencer()
{
	for(uint8_t i = 0; i < NUM_SEQ_TRACKS; i++)
	{
		gTracks[i].UpdateTrack();
	}

	gTracks[0].SetPlaying(gdpLoop1.IsActive());
	gTracks[1].SetPlaying(gdpLoop2.IsActive());
	gTracks[2].SetPlaying(gdpLoop3.IsActive());
	gTracks[3].SetPlaying(gdpLoop4.IsActive());

	bool pressed = gVirtualMuxPins[SEQ_VPIN_SEQUENCER_BTN].IsActive();
    gSeqEditBtn.UpdateState(pressed);
    if(gSeqEditBtn.IsPressed())
    {
        if(gCurrScreenPage == ScreenPage::SP_SEQUENCER_EDIT)
        {
			SetScreenPage(ScreenPage::SP_GENERAL_INFO);
        }
        else
        {
			SetScreenPage(ScreenPage::SP_SEQUENCER_EDIT);
			gSeqSelectStep = 0;
        }
    }

	if(gCurrScreenPage == ScreenPage::SP_SEQUENCER_EDIT)
	{

	}
}


/// @brief Get the sequencer page we are editing.
bool IsOnTrackEditPage()
{
	return gSeqSelectStep == 0;
}

/// @brief Get the track we are editing
SequencerTrack* GetCurrSequencerTrack()
{
	return &gTracks[gSeqSelectTrack];
}


/// @brief Get the step we are editing
SequencerStep* GetCurrSequencerStep(int8_t idx)
{
	GetCurrSequencerTrack()->mSteps[idx];
}


/// @brief Get the index of the currently selected sequencer track
int8_t GetCurrSequencerTrackIdx()
{
	return gSeqSelectTrack;
}

/// @brief Get the index of the currently selected sequencer track step.
int8_t GetCurrSequencerStepIdx()
{
	return gSeqSelectStep - 1;
}