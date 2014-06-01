#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "ch.h"
#include "hal.h"
#include "protocol.h"

extern uint8_t bl_wake;

uint8_t read_cmd(BaseChannel *chn);
uint8_t resetHandler(BaseChannel * chn);
uint8_t sendMode(BaseChannel * chn);
uint8_t wakeHandler(BaseChannel * chn);

#endif
