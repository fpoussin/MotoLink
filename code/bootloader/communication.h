#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "ch.h"
#include "hal.h"
#include "bootloader.h"
#include "protocol.h"

uint8_t read_cmd(BaseChannel *chn, uint8_t flags);
uint8_t writeHandler(BaseChannel *chn, uint8_t* buf, uint8_t len);
uint8_t readHandler(BaseChannel *chn, uint8_t* buf);
uint8_t eraseHandler(BaseChannel * chn, uint8_t* buf);
uint8_t resetHandler(BaseChannel * chn);
uint8_t sendFlags(BaseChannel * chn, uint8_t flags);
uint8_t sendMode(BaseChannel * chn);
uint8_t wakeHandler(BaseChannel * chn);
uint8_t bootHandler(BaseChannel * chn);

#endif
