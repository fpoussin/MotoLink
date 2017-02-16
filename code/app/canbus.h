#ifndef CANBUS_H
#define CANBUS_H

#include "hal.h"

void checkCanFilters(CANDriver *canp, const CANConfig *config);
void makeCanOBDFrame(CANTxFrame *txmsg, uint8_t pid);
void sendCanOBDFrames(CANDriver *canp, CANTxFrame *txmsg);
void readCanOBDPid(CANRxFrame *rxmsg);
void readCanYamahaPid(CANRxFrame *rxmsg);


#endif
