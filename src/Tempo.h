#include <Arduino.h>
#include <TimeInfo.h>

#ifndef TEMPO_H
#define TEMPO_H

void SetTempo(uint16_t tempo);

bool On4Note(uint8_t division = 1); // Quarter note and divisions

#endif // TEMPO_H