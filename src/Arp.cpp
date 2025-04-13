#include <Arp.h>
#include <Globals.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>
#include <FixedArray.h>

constexpr uint8_t ARP_NO_NOTE = 255;

bool gGoingUp = true;

uTimeMs gArpPlayingNoteTime = 0;
uint8_t gArpPlayingNote = ARP_NO_NOTE;

FixedArray<uint8_t, 8> gPressedNotesAbovePlayed;
FixedArray<uint8_t, 8> gPressedNotesBelowPlayed;

void ChooseNextNote(ArpMode mode, uint8_t lowestNote, uint8_t highestNote);

void PlayArp(ArpMode mode)
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
			gGoingUp = true;
		}
		// Go to next note on note off or we have none.
		else if (noteOff || gArpPlayingNote == ARP_NO_NOTE)
		{
			ChooseNextNote(mode, lowestNote, highestNote);
		}
	}

	if (noteOn && gArpPlayingNote != ARP_NO_NOTE && numPressed > 0)
	{
		MIDI.sendNoteOn(gArpPlayingNote, 100, 1);
	}
}



/// @brief Decide which note to set gArpPlayingNote to.
void ChooseNextNote(ArpMode mode, uint8_t lowestNote, uint8_t highestNote)
{
	long r = random(0,10);
	size_t aboveSize = gPressedNotesAbovePlayed.size();
	size_t belowSize = gPressedNotesBelowPlayed.size();

	switch (mode)
	{
	case ARP_RND:
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
	case ARP_UP:
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
	case ARP_DOWN:
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
	case ARP_UP_DOWN:
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
	default:
		//@TODO implement rest of arp modes.
		break;
	}
}