#include <MidiOutput.h>
#include <MIDI.h>
#include <VirtualPinToNote.h>
#include <Tempo.h>
#include <UserControls.h>

constexpr uint8_t METRONOME_NOTE = 75;

uint8_t gUpperCh = 1;
uint8_t gLowerCh = 1;
uint8_t gPlayVelocity = 100;
uint8_t gPlayingMetronomeNote = 0;
MIDI_CREATE_DEFAULT_INSTANCE();


void MidiOutputSetup()
{
	MIDI.begin(MIDI_CHANNEL_OFF);
}

void MidiOutputReadPins()
{

}

uint8_t GetChannel(bool upper)
{
    return upper ? gUpperCh : gLowerCh;
}

void SendNoteOn(uint8_t keyNum)
{
    bool isUpper = IsUpperKey(keyNum);
    uint8_t noteNum = KeyNumToNote(keyNum, 0, 0);
    MIDI.sendNoteOn(noteNum, gPlayVelocity, GetChannel(isUpper));
}

void SendNoteOff(uint8_t keyNum)
{
    bool isUpper = IsUpperKey(keyNum);
    uint8_t noteNum = KeyNumToNote(keyNum, 0, 0);
    MIDI.sendNoteOff(noteNum, gPlayVelocity, GetChannel(isUpper));
}

void SendNoteOn(uint8_t noteNum, bool upper)
{
    MIDI.sendNoteOn(noteNum, gPlayVelocity, GetChannel(upper));
}

void SendNoteOff(uint8_t noteNum, bool upper)
{
    MIDI.sendNoteOff(noteNum, 0, GetChannel(upper));
}

void SendNoteOnAllCh(uint8_t noteNum)
{
    for(uint8_t ch = 1; ch <= 16; ch++)
    {
        MIDI.sendNoteOn(noteNum, gPlayVelocity, ch);
    }
}

void SendNoteOffAllCh(uint8_t noteNum)
{
    for(uint8_t ch = 1; ch <= 16; ch++)
    {
        MIDI.sendNoteOff(noteNum, 0, ch);
    }
}

void PlayMetronome()
{
    bool noteOn = On4Note(1);
    bool noteOff = !noteOn && On4Note(2);
    uint8_t note = METRONOME_NOTE;

    if (noteOff && gPlayingMetronomeNote > 0)
    {
        SendNoteOffAllCh(note);
        gPlayingMetronomeNote = 0;
    }

    if (gdpMetronome.IsActive() && noteOn)
    {
        SendNoteOnAllCh(note);
        gPlayingMetronomeNote = note;
    }
}
