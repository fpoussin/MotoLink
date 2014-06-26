#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "ch.h"
#include "hal.h"
#include "protocol.h"
#include "sensors.h"
#include <string.h>

extern uint8_t bl_wake;

uint8_t readCommand(BaseChannel *chn);
uint8_t resetHandler(BaseChannel * chn);
uint8_t sendMode(BaseChannel * chn);
uint8_t wakeHandler(BaseChannel * chn);
uint8_t sendSensors(BaseChannel * chn);
uint8_t sendMonitoring(BaseChannel * chn);
uint8_t sendFFT(BaseChannel * chn);

#endif
