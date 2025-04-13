#include <Arp.h>
#include <Globals.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>
#include <FixedArray.h>

/// ===================================================================================
/// Constants
/// ===================================================================================
constexpr uint8_t ARP_NO_NOTE = 255;

/// ===================================================================================
/// Members
/// ===================================================================================
ArpMode gArpMode = ArpMode::ARP_OFF;
bool gGoingUp = true;

uTimeMs gArpPlayingNoteTime = 0;
uint8_t gArpPlayingNote = ARP_NO_NOTE;

FixedArray<uint8_t, 8> gPressedNotesAbovePlayed;
FixedArray<uint8_t, 8> gPressedNotesBelowPlayed;

/// ===================================================================================
/// Private function decl
/// ===================================================================================
void ChooseNextNote(uint8_t lowestNote, uint8_t highestNote);
void ResetState();


/// ===================================================================================
/// Public functions
/// ===================================================================================

/// @brief Is the arp enabled?
bool ArpEnabled()
{
	return gArpMode != ArpMode::ARP_OFF;
}



/// @brief Set arp mode.
void SetArpMode(ArpMode mode)
{
	ResetState();
	gArpMode = mode;
}



/// @brief Update notes and send out arp notes to midi
void PlayArp()
{
	// First inspect pressed keys.
	uint8_t numPressed = 0;
	
	uint8_t highestNote = 0;
	uint8_t lowestNote = 255;
	bool playingNextNote = false;

	gPressedNotesAbovePlayed.clear();
	gPressedNotesBelowPlayed.clear();

	for (int i = 0; i < NUM_NOTES; i++)
	{
		uint8_t vPinIdx = NOTES_VPIN_START + i;
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		uint8_t noteNum = VirtualPinToNote(i);
		
		gNoteStates[i].ChangeState(vPinState, gTime);

		bool pressed = gNoteStates[i].mPressed;

		if (pressed)
		{
			highestNote = max(highestNote, noteNum);
			lowestNote = min(lowestNote, noteNum);

			if (noteNum == gArpPlayingNote)
			{
				playingNextNote = true;
			}

			// Note: gArpPlayingNote might be none but that is fine.
			if (noteNum < gArpPlayingNote)
			{
				gPressedNotesBelowPlayed.insert_sorted(noteNum, true);
			}
			else if (noteNum > gArpPlayingNote)
			{
				gPressedNotesAbovePlayed.insert_sorted(noteNum, false);
			}

			numPressed++;
		}
	}

	// Now decide which notes to play
	bool note8 = On4Note(2);
	bool note16 = On4Note(4);

	bool noteOn = note8 && playingNextNote;
	bool noteOff = !noteOn && note16;

	if (noteOff)
	{
		if (gArpPlayingNote != ARP_NO_NOTE)
		{
			MIDI.sendNoteOff(gArpPlayingNote, 0, 1);
		}

		if (numPressed == 0)
		{
			gArpPlayingNote = ARP_NO_NOTE;
			ResetState();
		}
		// Go to next note on note off or we have none.
		else if (noteOff || gArpPlayingNote == ARP_NO_NOTE)
		{
			ChooseNextNote(lowestNote, highestNote);
		}
	}

	if (noteOn && gArpPlayingNote != ARP_NO_NOTE && numPressed > 0)
	{
		MIDI.sendNoteOn(gArpPlayingNote, 100, 1);
	}
}

/// ===================================================================================
/// Private functions
/// ===================================================================================

/// @brief Decide which note to set gArpPlayingNote to.
void ChooseNextNote(uint8_t lowestNote, uint8_t highestNote)
{
	long r = random(0,10);
	size_t aboveSize = gPressedNotesAbovePlayed.size();
	size_t belowSize = gPressedNotesBelowPlayed.size();

	switch (gArpMode)
	{
	case ARP_RND: // Play somewhat randomly
		if (r < 5 && aboveSize > 0) // Go above
		{
			r = random(0, aboveSize);
			gArpPlayingNote = gPressedNotesAbovePlayed[r];
		}
		else if (r < 10 && belowSize > 0) // Go below
		{
			r = random(0, belowSize);
			gArpPlayingNote = gPressedNotesBelowPlayed[r];
		}
		else // Go high or lowest notes?
		{
			gArpPlayingNote = r % 2 ? lowestNote : highestNote;
		}
		break;
	case ARP_UP: // Play ascending notes
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestNote;
		}
		else
		{
			if (aboveSize > 0)
			{
				// Go to next note above
				gArpPlayingNote = gPressedNotesAbovePlayed[0];
			}
			else
			{
				// Go back to lowest note.
				gArpPlayingNote = lowestNote;
			}
		}
		break;
	case ARP_UP_SPEC: // Play ascending notes in pairs of 2 // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestNote;
		}
		else
		{
			if (gGoingUp)
			{
				if (aboveSize > 1)
				{
					// Go to next note 2 above
					gArpPlayingNote = gPressedNotesAbovePlayed[1];
				}
				else
				{
					gArpPlayingNote = lowestNote;
				}
			}
			else
			{
				if (belowSize > 0)
				{
					// Go to next note 1 below
					gArpPlayingNote = gPressedNotesBelowPlayed[0];
				}
				else
				{
					gArpPlayingNote = highestNote;
				}
			}
			gGoingUp = !gGoingUp;
		}
		break;
	case ARP_DOWN: // Play descending notes // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = highestNote;
		}
		else
		{
			if (belowSize > 0)
			{
				// Go to next note above
				gArpPlayingNote = gPressedNotesBelowPlayed[0];
			}
			else
			{
				// Go back to lowest note.
				gArpPlayingNote = highestNote;
			}
		}
		break;
	case ARP_DOWN_SPEC: // Play descending notes in pairs of 2 // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = highestNote;
			gGoingUp = false;
		}
		else
		{
			if (gGoingUp)
			{
				if (aboveSize > 0)
				{
					// Go to next note 1 above
					gArpPlayingNote = gPressedNotesAbovePlayed[0];
				}
				else
				{
					gArpPlayingNote = lowestNote;
				}
			}
			else
			{
				if (belowSize > 1)
				{
					// Go to next note 2 below
					gArpPlayingNote = gPressedNotesBelowPlayed[1];
				}
				else
				{
					gArpPlayingNote = highestNote;
				}
			}
			gGoingUp = !gGoingUp;
		}
		break;
	case ARP_UP_DOWN: // Play ascending notes then descending notes
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestNote;
		}
		else
		{
			if (gGoingUp)
			{
				if (aboveSize > 0)
				{
					// Go to next note above
					gArpPlayingNote = gPressedNotesAbovePlayed[0];
				}
				else if (belowSize > 0)
				{
					// Start going down
					gGoingUp = false;
					gArpPlayingNote = gPressedNotesBelowPlayed[0];
				}
				else
				{
					gArpPlayingNote = highestNote;
				}
			}
			else
			{
				if (belowSize > 0)
				{
					// Go to next note below
					gArpPlayingNote = gPressedNotesBelowPlayed[0];
				}
				else if (aboveSize > 0)
				{
					// Start going up
					gGoingUp = true;
					gArpPlayingNote = gPressedNotesAbovePlayed[0];
				}
				else
				{
					gArpPlayingNote = lowestNote;
				}
			}
		}
		break;
	case ARP_UP_DOWN_SPEC: // Play ascending notes then descending notes with random variation // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestNote;
		}
		else
		{
			if (gGoingUp)
			{
				if (r % 2 && aboveSize > 1) // 50% chance to skip note
				{
					// Go to next note 2 above
					gArpPlayingNote = gPressedNotesAbovePlayed[1];
				}
				else if (aboveSize > 0)
				{
					// Go to next note above
					gArpPlayingNote = gPressedNotesAbovePlayed[0];
				}
				else if (belowSize > 0)
				{
					// Start going down
					gGoingUp = false;
					gArpPlayingNote = gPressedNotesBelowPlayed[0];
				}
				else
				{
					gArpPlayingNote = highestNote;
				}
			}
			else
			{
				if (r % 2 && belowSize > 1) // 50% chance to skip note
				{
					// Go to next note 2 above
					gArpPlayingNote = gPressedNotesBelowPlayed[1];
				}
				else if (belowSize > 0)
				{
					// Go to next note below
					gArpPlayingNote = gPressedNotesBelowPlayed[0];
				}
				else if (aboveSize > 0)
				{
					// Start going up
					gGoingUp = true;
					gArpPlayingNote = gPressedNotesAbovePlayed[0];
				}
				else
				{
					gArpPlayingNote = lowestNote;
				}
			}
		}
		break;
	}
}

/// @brief Reset state.
void ResetState()
{
	if (gArpPlayingNote != ARP_NO_NOTE)
	{
		MIDI.sendNoteOff(gArpPlayingNote, 0, 1);
	}
	gPressedNotesAbovePlayed.clear();
	gPressedNotesBelowPlayed.clear();
	gGoingUp = true;
	gArpPlayingNoteTime = gTime;
}