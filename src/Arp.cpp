#include <Arp.h>
#include <Globals.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>
#include <FixedArray.h>
#include <MidiOutput.h>
#include <UserControls.h>

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



/// @brief Set arp mode based on pins
void ReadArpMode()
{	
	uint8_t mode = (uint8_t)gdpArpUp.IsActive();
	mode |= (uint8_t)(gdpArpDown.IsActive() << 1);
	mode |= (uint8_t)(gdpArpSpec.IsActive() << 2);

	if (mode != gArpMode)
	{
		ResetState();
	}

	gArpMode = (ArpMode)mode;
}



/// @brief Update notes and send out arp notes to midi
void PlayArp()
{
	// First inspect pressed keys.
	uint8_t numPressed = 0;
	
	uint8_t highestKey = 0;
	uint8_t lowestKey = 255;

	gPressedNotesAbovePlayed.clear();
	gPressedNotesBelowPlayed.clear();

	for (int i = 0; i < NUM_NOTES; i++)
	{
		uint8_t vPinIdx = NOTES_VPIN_START + i;
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		uint8_t keyNum = VirtualPinToKeyNum(i);
		
		gNoteStates[i].ChangeState(vPinState, gTime);

		bool pressed = gNoteStates[i].mPressed;

		if (pressed)
		{
			highestKey = max(highestKey, keyNum);
			lowestKey = min(lowestKey, keyNum);

			// Note: gArpPlayingNote might be none but that is fine.
			if (keyNum < gArpPlayingNote)
			{
				gPressedNotesBelowPlayed.insert_sorted(keyNum, true);
			}
			else if (keyNum > gArpPlayingNote)
			{
				gPressedNotesAbovePlayed.insert_sorted(keyNum, false);
			}

			numPressed++;
		}
	}

	// Nothing pressed, turn off arp now.
	if (numPressed == 0)
	{
		ResetState();
		return;
	}

	// Now decide which notes to play
	bool note8 = On4Note(2);
	bool note16 = On4Note(4);

	bool noteOn = note8;
	bool noteOff = !noteOn && note16;

	if (noteOn)
	{
		ChooseNextNote(lowestKey, highestKey);
		SendNoteOn(gArpPlayingNote);
	}
	else if (noteOff)
	{
		if (gArpPlayingNote != ARP_NO_NOTE)
		{
			SendNoteOff(gArpPlayingNote);
		}
	}
}

/// ===================================================================================
/// Private functions
/// ===================================================================================

/// @brief Decide which note to set gArpPlayingNote to.
void ChooseNextNote(uint8_t lowestKey, uint8_t highestKey)
{
	long r = random(0,10);
	size_t aboveSize = gPressedNotesAbovePlayed.size();
	size_t belowSize = gPressedNotesBelowPlayed.size();

	switch (gArpMode)
	{
	case ARP_OFF:
	break;
	case ARP_SPEC: // Play somewhat randomly
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
			gArpPlayingNote = r % 2 ? lowestKey : highestKey;
		}
		break;
	case ARP_UP: // Play ascending notes
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestKey;
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
				gArpPlayingNote = lowestKey;
			}
		}
		break;
	case ARP_UP_SPEC: // Play ascending notes in pairs of 2 // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestKey;
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
					gArpPlayingNote = lowestKey;
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
					gArpPlayingNote = highestKey;
				}
			}
			gGoingUp = !gGoingUp;
		}
		break;
	case ARP_DOWN: // Play descending notes // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = highestKey;
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
				gArpPlayingNote = highestKey;
			}
		}
		break;
	case ARP_DOWN_SPEC: // Play descending notes in pairs of 2 // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = highestKey;
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
					gArpPlayingNote = lowestKey;
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
					gArpPlayingNote = highestKey;
				}
			}
			gGoingUp = !gGoingUp;
		}
		break;
	case ARP_UP_DOWN: // Play ascending notes then descending notes
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestKey;
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
					gArpPlayingNote = highestKey;
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
					gArpPlayingNote = lowestKey;
				}
			}
		}
		break;
	case ARP_UP_DOWN_SPEC: // Play ascending notes then descending notes with random variation // NOT TESTED
		if (gArpPlayingNote == ARP_NO_NOTE)
		{
			gArpPlayingNote = lowestKey;
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
					gArpPlayingNote = highestKey;
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
					gArpPlayingNote = lowestKey;
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
		SendNoteOff(gArpPlayingNote);
		gArpPlayingNote = ARP_NO_NOTE;
	}
	gPressedNotesAbovePlayed.clear();
	gPressedNotesBelowPlayed.clear();
	gGoingUp = true;
	gArpPlayingNoteTime = gTime;
}