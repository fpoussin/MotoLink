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
uint8_t rpcReadRequest(BaseChannel * chn);
uint8_t rpcSendResponse(BaseChannel * chn, uint32_t cmd, uint32_t code, const uint8_t *data);
uint8_t rpcReadData(BaseChannel * chn, uint8_t * data, uint8_t data_len);
uint8_t rpcSendData(BaseChannel * chn, uint8_t * data, uint8_t data_len);


#endif
