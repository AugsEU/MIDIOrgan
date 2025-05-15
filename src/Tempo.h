#include <Arduino.h>
#include <Util/TimeInfo.h>

#ifndef TEMPO_H
#define TEMPO_H

extern uTimeMs gTempoInterval;
extern uint16_t gTempoCache;

void UpdateTempo();


void RecalculateTempo();
void SetTempo();

bool On4Note(uint8_t division = 1); // Quarter note and divisions

#endif // TEMPO_H