#ifndef RPC_H
#define RPC_H

#include "hal.h"
#include "protocol.h"
#include "proto/base.pb.h"
#include "proto/bootloader.pb.h"
#include "proto/app.pb.h"


/* Typedefs */
typedef void (*pCommand)(uint8_t *data, uint16_t *data_len);


/* RPC */
void readRequest(BaseChannel * chn, BaseRequest * req);
void sendResponse(BaseChannel * chn, uint8_t code);
void sendResponseWithData(BaseChannel * chn, uint8_t code, uint8_t * data, uint8_t data_len);


#endif
