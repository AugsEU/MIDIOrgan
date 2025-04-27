#include <Arduino.h>
#include <TimeInfo.h>

#ifndef TEMPO_H
#define TEMPO_H

constexpr uint16_t DEFAULT_TEMPO = 120;

extern uTimeMs gTempoInterval;

void UpdateTempo();

void SetTempo(uint16_t tempo);

bool On4Note(uint8_t division = 1); // Quarter note and divisions

#endif // TEMPO_H