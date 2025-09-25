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
constexpr uint8_t SEQ_VPIN_SEQUENCER_BTN = 1;
constexpr uTimeMs SEQ_MAX_RELEASE_TIME = 300;// 300ms

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
	ClearNotes();
	mVelocity = 99;
	mLength = 1;
}

/// @brief Clear all notes
void SequencerStep::ClearNotes()
{
	memset(mNotes, 0, STEP_POLYPHONY);
	mLength &= 0x7F; // unlock notes
}

void SequencerStep::AddNote(uint8_t note)
{
	if(NotesLocked())
	{
		ClearNotes();
		mNotes[0] = note;
		return;
	}

	uint8_t i = 0;
	while(true)
	{
		if(note == mNotes[i] || i == STEP_POLYPHONY)
		{
			return;
		}
		if(mNotes[i] == 0)
		{
			break;
		}
		i++;
	}
	
	mNotes[i] = note;
}

uint8_t SequencerStep::GetLength()
{
	return mLength & 0x7F;
}

void SequencerStep::SetLength(uint8_t len)
{
	if(len == 0) len = 1;
	mLength = (mLength & 0x80) | len;
}

bool SequencerStep::NotesLocked()
{
	return (mLength & 0x80) != 0;
}

void SequencerStep::LockNotes()
{
	mLength |= 0x80;
}

/// ===================================================================================
/// SequencerTrack
/// ===================================================================================

/// @brief Init sequencer track.
SequencerTrack::SequencerTrack()
{
	mPlaying = false;
	mMidiCh = 0;
	mSubDiv = 1;
	mNumSteps = 16;

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
		WritePlayNoteBuf(note, step->GetLength());
	}
}

/// @brief Set the track to be playing or not.
void SequencerTrack::SetPlaying(bool playing)
{
	bool isStopping = !playing;
	bool canStartPlaying = !mPlaying && On4Note(1);
	if(isStopping || canStartPlaying)
	{
		if(playing != mPlaying)
		{
			mPlayHead = 0;// Reset playhead when stopping or starting.
		}
		mPlaying = playing;
	}
}

/// ===================================================================================
/// Static functions
/// ===================================================================================
void UpdateTrackEdit(SequencerTrack* currTrack);
void UpdateStepEdit(SequencerTrack* currTrack);

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
		SequencerTrack* currTrack = GetCurrSequencerTrack();

		int8_t stepDelta = gRotaryEncoders[SEQ_PAGE_SELECT_DIAL_IDX].ConsumeDelta();
		gSeqSelectStep += stepDelta;

		// Clamp
		if(gSeqSelectStep < 0) gSeqSelectStep = 0;
		else if(gSeqSelectStep >= currTrack->mNumSteps) gSeqSelectStep = currTrack->mNumSteps;

		if(IsOnTrackEditPage())
		{
			UpdateTrackEdit(currTrack);
		}
		else
		{
			UpdateStepEdit(currTrack);
		}
	}
}


/// @brief Update the track edit page
void UpdateTrackEdit(SequencerTrack* currTrack)
{
	EnableScreenCursor(false);
	currTrack->mSubDiv = ApplyDelta(currTrack->mSubDiv, gRotaryEncoders[SEQ_TOP_RIGHT_DIAL].ConsumeDelta(), 16);
	if(currTrack->mSubDiv == 0)
	{
		currTrack->mSubDiv = 1;
	}
	currTrack->mMidiCh = ApplyDelta(currTrack->mMidiCh, gRotaryEncoders[SEQ_BOT_LEFT_DIAL].ConsumeDelta(), 16);
	currTrack->mNumSteps = ApplyDelta(currTrack->mNumSteps, gRotaryEncoders[SEQ_BOT_RIGHT_DIAL].ConsumeDelta(), TRACK_STEP_MAX);
	if(currTrack->mNumSteps == 0)
	{
		currTrack->mNumSteps = 1;
	}
	gSeqSelectTrack = ApplyDelta(gSeqSelectTrack, gRotaryEncoders[SEQ_TOP_LEFT_DIAL].ConsumeDelta(), 3);
}


/// @brief Update the step edit page
void UpdateStepEdit(SequencerTrack* currTrack)
{
	int8_t stepIdx = GetCurrSequencerStepIdx();
    int8_t page = stepIdx >> 4; //div 16
    int8_t subPageIdx = stepIdx - (page<<4);
	SequencerStep* step = &currTrack->mSteps[stepIdx];

	EnableScreenCursor(true);
	PlaceScreenCursor(subPageIdx, 0);

	step->mVelocity = ApplyDelta(step->mVelocity, gRotaryEncoders[SEQ_BOT_LEFT_DIAL].ConsumeDelta(), 99);
	uint8_t newLen = ApplyDelta(step->GetLength(), gRotaryEncoders[SEQ_BOT_RIGHT_DIAL].ConsumeDelta(), 16);
	step->SetLength(newLen);

	// Clear notes
	if(gRotaryEncoders[SEQ_TOP_LEFT_DIAL].ConsumeDelta() != 0 ||
		gRotaryEncoders[SEQ_TOP_RIGHT_DIAL].ConsumeDelta() != 0 ||
		gdpLoopClear.IsActive())
	{
		step->ClearNotes();
	}

	// Find notes we are pressing
	uint8_t numNotesPressed = 0;
	for(uint8_t key = 0; key < NUM_NOTES; key++)
	{
		if(gNoteStates[key].mState == NPS_PRESSED)
		{
			uint8_t note = KeyNumToNote(key);
			step->AddNote(note);

			numNotesPressed++;
			if(step->mNotes[STEP_POLYPHONY-1] != 0)
			{
				break;
			}
		}
	}

	if(numNotesPressed == 0)
	{
		// When all notes are released we lock the notes. Next time a note is added it will erase the rest.
		step->LockNotes();
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
	return &GetCurrSequencerTrack()->mSteps[idx];
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