#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "hal.h"

extern uint8_t bl_wake;

uint8_t readCommand(BaseChannel *chn);
uint8_t resetHandler(BaseChannel *chn);
uint8_t sendMode(BaseChannel *chn);
uint8_t wakeHandler(BaseChannel *chn);
uint8_t sendSensors(BaseChannel *chn);
uint8_t sendMonitoring(BaseChannel *chn);
uint8_t sendFFT(BaseChannel *chn);
uint8_t writeSettings(BaseChannel *chn, uint8_t *buf, uint16_t len);
uint8_t readSettings(BaseChannel *chn);
uint8_t writeHeaders(BaseChannel *chn, uint8_t *buf, uint16_t len);
uint8_t readHeaders(BaseChannel *chn);
uint8_t writeTables(BaseChannel *chn, uint8_t *buf, uint16_t len);
uint8_t readTables(BaseChannel *chn);
uint8_t clearCell(BaseChannel *chn, uint8_t *buf, uint16_t len);
uint8_t clearTables(BaseChannel *chn);
uint8_t sendVersion(BaseChannel *chn);

#endif
