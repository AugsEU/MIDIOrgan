#include <Arduino.h>
#include <TimeInfo.h>

#ifndef TEMPO_H
#define TEMPO_H

void SetTempo(uint16_t tempo);

bool On4Note(); // Quarter note
bool On8Note(); // Eight note

#endif // TEMPO_H