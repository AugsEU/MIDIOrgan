// #include <Arp.h>
// #include <Constants.h>
// #include <StableState.h>
// #include <NotePressInfo.h>
// #include <VirtualPinToNote.h>
// #include <Tempo.h>
// #include <MIDI.h>
// #include <Globals.h>

// uint8_t gArpStep = 0;

// void PlayArpUp();

// void Arp::PlayNotes()
// {
//     switch (mMode)
//     {
//     case ARP_UP:
//         PlayArpUp();
//         break;
//     default:
//         break;
//     }
// }

// void PlayArpUp()
// {
//     bool note4 = On4Note(gPrevTime.mTimeMs, gTime.mTimeMs);
//     bool note8 = On8Note(gPrevTime.mTimeMs, gTime.mTimeMs);

//     bool noteOn = note4;
//     bool noteOff = !note4 && note8;

//     if (!note4 && !note8)
//     {
//         return;
//     }

//     uint8_t numPressed = 0;

// 	for (int i = 0; i < NUM_NOTES; i++)
// 	{
// 		uint8_t vPinIdx = NOTES_VPIN_START + i;
// 		bool vPinState = gVirtualMuxPins[vPinIdx].IsActive();
// 		uint8_t noteNum = VirtualPinToNote(i);

//         if (vPinState)
//         {
//             MIDI.sendNoteOn(noteNum, 100, 1);
//         }
		
// 		gNoteStates[i].ChangeState(vPinState, gTime.mTimeMs);

// 		bool pressed = gNoteStates[i].mPressed;

// 		if (pressed)
// 		{
//             MIDI.sendNoteOn(noteNum, 100, 1);
//             if (numPressed == gArpStep)
//             {
//                 if (noteOn)
//                 {
                    
//                 }
//                 else if (noteOff)
//                 {
//                     //MIDI.sendNoteOff(noteNum, 100, 1);
//                 }
//             }
//             numPressed++;
// 		}
// 	}

//     if (noteOff)
//     {
//         gArpStep++;
//     }
// }
