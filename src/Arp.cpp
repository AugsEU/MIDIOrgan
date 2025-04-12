#include <Arp.h>
#include <Globals.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>

uint8_t gArpStep = 0;

void PlayArpUp();

void PlayArp(ArpMode mode)
{
    switch (mode)
    {
    case ARP_UP:
        PlayArpUp();
        break;
    default:
        break;
    }
}

void PlayArpUp()
{
    bool note4 = On4Note();
    bool note8 = On8Note();

    bool noteOn = note4;
    bool noteOff = !note4 && note8;

    if (!note4 && !note8)
    {
        return;
    }

    uint8_t numPressed = 0;

	for (int i = 0; i < NUM_NOTES; i++)
	{
		uint8_t vPinIdx = NOTES_VPIN_START + i;
		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
		uint8_t noteNum = VirtualPinToNote(i);
		
		gNoteStates[i].ChangeState(vPinState, gTime);

		bool pressed = gNoteStates[i].mPressed;

		if (pressed)
		{
            if (numPressed == gArpStep)
            {
                if (noteOn)
                {
					MIDI.sendNoteOn(noteNum, 100, 1); 
                }
                else if (noteOff)
                {
                    MIDI.sendNoteOff(noteNum, 100, 1);
                }
            }
            numPressed++;
		}
	}

    if (noteOff)
    {
        gArpStep++;
    }

	if (gArpStep >= numPressed)
	{
		gArpStep = 0;
	}
}
