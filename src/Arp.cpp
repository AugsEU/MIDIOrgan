#include <Arp.h>
#include <Globals.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>
#include <FixedArray.h>
#include <MidiOutput.h>
#include <UserControls.h>
#include <StableAnalog.h>

/// ===================================================================================
/// Constants
/// ===================================================================================


/// ===================================================================================
/// Members
/// ===================================================================================
ArpMode gArpMode = ArpMode::ARP_OFF;
ArpSpeed gArpSpeed = ArpSpeed::ARP_SPEED_EIGHTH;

StableAnalog gArpGate;

Arpeggiator gLowerArp;
Arpeggiator gUpperArp;

FixedArray<uint8_t, 8> gPressedNotesAbovePlayed;
FixedArray<uint8_t, 8> gPressedNotesBelowPlayed;


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
		gLowerArp.ResetState();
		gUpperArp.ResetState();
	}

	gArpMode = (ArpMode)mode;

	uint8_t speed = (uint8_t)gdpArpSlow.IsActive() | ((uint8_t)gdpArpFast.IsActive() << 1);
	gArpSpeed = (ArpSpeed)speed;

	gArpGate.ConsumeInput(gapArpGate);
}



/// @brief Update notes and send out arp notes to midi
void PlayArp()
{
	if (gdpArpSelectLower.IsActive())
	{
		gLowerArp.PlayNotes(0, NUM_LOWER_KEYS);
	}
	
	if (gdpArpSelectUpper.IsActive())
	{
		gUpperArp.PlayNotes(NUM_LOWER_KEYS+1, NUM_NOTES);
	}
}

/// ===================================================================================
/// Arpeggiator
/// ===================================================================================

void Arpeggiator::PlayNotes(uint8_t keyStart, uint8_t keyEnd)
{
	// First inspect pressed keys.
	uint8_t numPressed = 0;

	uint8_t highestKey = 0;
	uint8_t lowestKey = 255;

	gPressedNotesAbovePlayed.clear();
	gPressedNotesBelowPlayed.clear();

	for (uint8_t i = keyStart; i < keyEnd; i++)
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

			// Note: mPlayingKey might be none but that is fine.
			if (keyNum < mPlayingKey)
			{
				gPressedNotesBelowPlayed.insert_sorted(keyNum, true);
			}
			else if (keyNum > mPlayingKey)
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
	if (mPlayingKey == NOTE_NONE)
	{
		// Do not start arp until on beat.
		if (!On4Note(2))
		{
			return;
		}
	}

	bool noteOn = false;
	switch (gArpSpeed)
	{
	case ArpSpeed::ARP_SPEED_QUARTER:
		noteOn = On4Note(2);
		break;
	case ArpSpeed::ARP_SPEED_EIGHTH:
		noteOn = On4Note(4);
		break;
	case ArpSpeed::ARP_SPEED_TRIPLET:
		noteOn = On4Note(6);
		break;
	case ArpSpeed::ARP_SPEED_SIXTEENTH:
		noteOn = On4Note(8);
		break;
	default:
		break;
	}

	if (noteOn)
	{
		ChooseNextNote(lowestKey, highestKey);
		SendNoteOn(mPlayingKey);
		mLastNoteTime = gTime;
	}
	
	uTimeMs gateTime = 0; 
	switch (gArpSpeed)
	{
	case ArpSpeed::ARP_SPEED_QUARTER:
		gateTime = gTempoInterval / 2;
		break;
	case ArpSpeed::ARP_SPEED_EIGHTH:
		gateTime = gTempoInterval / 4;
		break;
	case ArpSpeed::ARP_SPEED_TRIPLET:
		gateTime = gTempoInterval / 6;
		break;
	case ArpSpeed::ARP_SPEED_SIXTEENTH:
		gateTime = gTempoInterval / 8;
		break;
	default:
		break;
	}

	gateTime -= 1;
	gateTime *= gArpGate.mStableValue;
	gateTime >>= ANALOG_READ_RESOLUTION_BITS;
	bool noteOff = gTime > mLastNoteTime + gateTime;

	if (noteOff)
	{
		if (mPlayingKey != NOTE_NONE)
		{
			SendNoteOff(mPlayingKey);
		}
	}
}

/// @brief Decide which note to set mPlayingNote to.
void Arpeggiator::ChooseNextNote(uint8_t lowestKey, uint8_t highestKey)
{
	long r = random(0,10);
	size_t aboveSize = gPressedNotesAbovePlayed.size();
	size_t belowSize = gPressedNotesBelowPlayed.size();

	if (mPlayingKey == NOTE_NONE || (aboveSize == 0 && belowSize == 0))
	{
		// Define first note:
		switch (gArpMode)
		{
		case ARP_OFF:
			break;
		case ARP_SPEC: // Play somewhat randomly
			mPlayingKey = r % 2 ? lowestKey : highestKey;
			break;
		case ARP_UP:
			mPlayingKey = lowestKey;
			break;
		case ARP_UP_SPEC:
			mPlayingKey = lowestKey;
			break;
		case ARP_DOWN:
			mPlayingKey = highestKey;
			break;
		case ARP_DOWN_SPEC:
			mPlayingKey = highestKey;
			mGoingUp = false;
			break;
		case ARP_UP_DOWN:
			mPlayingKey = lowestKey;
			break;
		case ARP_UP_DOWN_SPEC:
			mPlayingKey = lowestKey;
			break;
		}
		return;
	}

	switch (gArpMode)
	{
	case ARP_OFF:
	break;
	case ARP_SPEC: // Play somewhat randomly
		if ((r < 5 || belowSize == 0) && aboveSize > 0) // Go above
		{
			r = random(0, aboveSize);
			mPlayingKey = gPressedNotesAbovePlayed[r];
		}
		else // Go below
		{
			r = random(0, belowSize);
			mPlayingKey = gPressedNotesBelowPlayed[r];
		}
		break;
	case ARP_UP: // Play ascending notes
		if (aboveSize > 0)
		{
			// Go to next note above
			mPlayingKey = gPressedNotesAbovePlayed[0];
		}
		else
		{
			// Go back to lowest note.
			mPlayingKey = lowestKey;
		}
		break;
	case ARP_UP_SPEC: // Play ascending notes in pairs of 2
		if (aboveSize == 0)
		{
			mPlayingKey = lowestKey;
			mGoingUp = true;
		}
		else if (mGoingUp)
		{
			if (aboveSize > 1)
			{
				// Go to next note 2 above
				mPlayingKey = gPressedNotesAbovePlayed[1];
			}
			else
			{
				mPlayingKey = gPressedNotesAbovePlayed[0];
			}

			mGoingUp = false;
		}
		else
		{
			if (belowSize > 0)
			{
				// Go to next note 1 below
				mPlayingKey = gPressedNotesBelowPlayed[0];
			}
			else
			{
				mPlayingKey = lowestKey;
			}
			mGoingUp = true;
		}
		break;
	case ARP_DOWN: // Play descending notes // NOT TESTED
		if (belowSize > 0)
		{
			// Go to next note above
			mPlayingKey = gPressedNotesBelowPlayed[0];
		}
		else
		{
			// Go back to lowest note.
			mPlayingKey = highestKey;
		}
		break;
	case ARP_DOWN_SPEC: // Play descending notes in pairs of 2 // NOT TESTED
		if (belowSize == 0)
		{
			mPlayingKey = highestKey;
			mGoingUp = false;
		}
		else if (!mGoingUp)
		{
			if (belowSize > 1)
			{
				// Go to next note 2 above
				mPlayingKey = gPressedNotesBelowPlayed[1];
			}
			else
			{
				mPlayingKey = gPressedNotesBelowPlayed[0];
			}

			mGoingUp = true;
		}
		else
		{
			if (aboveSize > 0)
			{
				// Go to next note 1 below
				mPlayingKey = gPressedNotesAbovePlayed[0];
			}
			else
			{
				mPlayingKey = highestKey;
			}
			mGoingUp = false;
		}
		break;
	case ARP_UP_DOWN: // Play ascending notes then descending notes
		if (mGoingUp)
		{
			if (aboveSize > 0)
			{
				// Go to next note above
				mPlayingKey = gPressedNotesAbovePlayed[0];
			}
			else if (belowSize > 0)
			{
				// Start going down
				mGoingUp = false;
				mPlayingKey = gPressedNotesBelowPlayed[0];
			}
			else
			{
				mPlayingKey = highestKey;
			}
		}
		else
		{
			if (belowSize > 0)
			{
				// Go to next note below
				mPlayingKey = gPressedNotesBelowPlayed[0];
			}
			else if (aboveSize > 0)
			{
				// Start going up
				mGoingUp = true;
				mPlayingKey = gPressedNotesAbovePlayed[0];
			}
			else
			{
				mPlayingKey = lowestKey;
			}
		}
		break;
	case ARP_UP_DOWN_SPEC: // Play ascending notes then descending notes with random variation // NOT TESTED
		if (mGoingUp)
		{
			if (r % 2 && aboveSize > 1) // 50% chance to skip note
			{
				// Go to next note 2 above
				mPlayingKey = gPressedNotesAbovePlayed[1];
			}
			else if (aboveSize > 0)
			{
				// Go to next note above
				mPlayingKey = gPressedNotesAbovePlayed[0];
			}
			else if (belowSize > 0)
			{
				// Start going down
				mGoingUp = false;
				mPlayingKey = gPressedNotesBelowPlayed[0];
			}
			else
			{
				mPlayingKey = highestKey;
			}
		}
		else
		{
			if (r % 2 && belowSize > 1) // 50% chance to skip note
			{
				// Go to next note 2 above
				mPlayingKey = gPressedNotesBelowPlayed[1];
			}
			else if (belowSize > 0)
			{
				// Go to next note below
				mPlayingKey = gPressedNotesBelowPlayed[0];
			}
			else if (aboveSize > 0)
			{
				// Start going up
				mGoingUp = true;
				mPlayingKey = gPressedNotesAbovePlayed[0];
			}
			else
			{
				mPlayingKey = lowestKey;
			}
		}
		break;
	}
}

/// @brief Reset state.
void Arpeggiator::ResetState()
{
	if (mPlayingKey != NOTE_NONE)
	{
		SendNoteOff(mPlayingKey);
		mPlayingKey = NOTE_NONE;
	}
	
	mGoingUp = true;
	mLastNoteTime = gTime;
}