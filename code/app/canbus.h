#ifndef CANBUS_H
#define CANBUS_H

#include "hal.h"

void checkCanFilters(CANDriver *canp, const CANConfig *config);

void serveCanOBDPidRequest(CANDriver *canp, CANTxFrame *txmsg, const CANRxFrame *rxmsg);

void makeCanOBDPidRequest(CANTxFrame *txmsg, uint8_t pid);
void sendCanOBDFrames(CANDriver *canp, CANTxFrame *txmsg);
void readCanOBDPidResponse(const CANRxFrame *rxmsg);

void readCanYamahaPid(const CANRxFrame *rxmsg);


#endif
