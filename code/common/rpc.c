#include "rpc.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "common.h"
#include "rpc_commands.h"

uint8_t rpcReadRequest(BaseChannel * chn) {

  uint8_t buffer[BaseRequest_size];
  uint32_t crc_buffer;
  BaseRequest request = BaseRequest_init_default;
  pb_istream_t input = pb_istream_from_buffer(buffer, BaseRequest_size);
  uint8_t status = 0;

  if (chnReadTimeout(chn, buffer, BaseRequest_size, MS2ST(50)) < BaseHeader_size)
  {
    rpcSendResponse(chn, 0, 1, 0);
    return 1;
  }

  if (chnReadTimeout(chn, (uint8_t*)&crc_buffer, sizeof(crc_buffer), MS2ST(10)) < sizeof(crc_buffer))
  {
    rpcSendResponse(chn, 0, 2, 0);
    return 2;
  }

  if (getCrc(buffer, BaseRequest_size) != crc_buffer)
  {
    rpcSendResponse(chn, 0, 6, 0);
    return 3;
  }

  if (!pb_decode(&input, BaseRequest_fields, &request))
  {
    rpcSendResponse(chn, 0, 4, 0);
    return 4;
  }

  if (strcmp(request.header.magic, MAGICSTR) != 0) {
    rpcSendResponse(chn, 0, 5, 0);
    return 5;
  }

  // Inform function is decoded, then let it handle the rest.
  rpcSendResponse(chn, 0, 0, 0);
  switch (request.header.cmd)
  {

    default:
      status = 3;
      break;
  }

  return status;
}

uint8_t rpcSendResponse(BaseChannel * chn, uint32_t cmd, uint32_t code, const uint8_t *data)
{
  uint8_t buffer[BaseResponse_size];
  BaseResponse response = BaseResponse_init_default;

  strcpy(response.header.magic, MAGICSTR);
  response.header.cmd = cmd;
  response.code = code;
  if (data != NULL)
    memcpy(response.data.bytes, data, 4);
  pb_ostream_t output = pb_ostream_from_buffer(buffer, BaseResponse_size);

  if (!pb_encode(&output, BaseResponse_fields, &response))
  {
      return 1;
  }

  chnWriteTimeout(chn, buffer, sizeof(buffer), MS2ST(50));

  return 0;
}


uint8_t rpcSendData(BaseChannel *chn, uint8_t *data, uint8_t data_len)
{

  return 0;
}

uint8_t rpcReadData(BaseChannel *chn, uint8_t *data, uint8_t data_len)
{

  return 0;
}
