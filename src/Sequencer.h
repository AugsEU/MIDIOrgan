#include <Arduino.h>

#ifndef SEQUENCER_H
#define SEQUENCER_H

constexpr uint8_t STEP_POLYPHONY = 6;
constexpr uint8_t TRACK_STEP_MAX = 96;
constexpr uint8_t NUM_SEQ_TRACKS = 4;

extern int8_t gSeqSelectTrack;
extern int8_t gSeqSelectStep;

struct SequencerStep
{
	uint8_t mNotes[STEP_POLYPHONY];
	uint8_t mVelocity;

	SequencerStep();
	void ClearNotes();
	void AddNote(uint8_t note);
	uint8_t GetLength();
	void SetLength(uint8_t len);
	bool NotesLocked();
	void LockNotes();

private:
	uint8_t mLength; // High bit of length is used for note lock to save memory.
};

struct SequencerTrack
{
	bool mPlaying;
	uint8_t mMidiCh;
	uint8_t mSubDiv;
	uint8_t mNumSteps;
	
	uint8_t mPlayHead;
	SequencerStep mSteps[TRACK_STEP_MAX];
	uint8_t mPlayingNotes[64];

	SequencerTrack();
	void EraseAllSteps();
	
	void WritePlayNoteBuf(uint8_t note, uint8_t duration);
	uint8_t ReadPlayNoteBuf(uint8_t note);

	void UpdateTrack();

	void PlayStep(SequencerStep* step);

	void SetPlaying(bool playing);
};

void InitSequencer();
void UpdateSequencer();

bool IsOnTrackEditPage();
SequencerTrack* GetCurrSequencerTrack();
SequencerStep* GetCurrSequencerStep(int8_t idx);
int8_t GetCurrSequencerTrackIdx();
int8_t GetCurrSequencerStepIdx();

#endif // SEQUENCER_H
