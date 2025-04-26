#include <Arduino.h>
#include <TimeInfo.h>
#include <NotePressInfo.h>
#include <Constants.h>

#ifndef GLOBALS_H
#define GLOBALS_H

extern uTimeMs gTime;
extern uTimeMs gPrevTime;
extern NotePressInfo gNoteStates[NUM_NOTES];

#endif // GLOBALS_H