#ifndef RPC_H
#define RPC_H

#include "hal.h"
#include "protocol.h"
#include "proto/base.pb.h"
#include "proto/bootloader.pb.h"
#include "proto/app.pb.h"


/* Typedefs for callbacks */
typedef void (*pCommand)(uint8_t *data, uint16_t *data_len);


/* RPC */
uint8_t readRequest(BaseChannel * chn);
uint8_t sendResponse(BaseChannel * chn, uint8_t code);
uint8_t ReadData(BaseChannel * chn, uint8_t * data, uint8_t data_len);
uint8_t sendData(BaseChannel * chn, uint8_t * data, uint8_t data_len);


#endif
