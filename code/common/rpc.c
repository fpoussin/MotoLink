#include "rpc.h"
#include "pb_encode.h"
#include "pb_decode.h"

uint8_t readRequest(BaseChannel * chn) {

  uint8_t buffer[BaseRequest_size];
  BaseRequest request = BaseRequest_init_default;
  pb_istream_t input = pb_istream_from_buffer(buffer, BaseRequest_size);
  uint8_t status = 0;

  if (chnReadTimeout(chn, buffer, BaseRequest_size, MS2ST(50)) < BaseHeader_size)
  {
    return 1;
  }

  if (!pb_decode(&input, BaseRequest_fields, &request))
  {
    return 2;
  }

  if (strcmp(request.header.magic, MAGICSTR) != 0)
    return 3;

  //TODO: Check CRC
  //

  switch (request.header.cmd)
  {

    default:
      status = 3;
      break;
  }

  return status;
}


uint8_t sendResponse(BaseChannel *chn, uint8_t code)
{
  uint8_t buffer[BaseResponse_size];
  BaseResponse response = BaseResponse_init_default;

  response.code = code;
  pb_ostream_t output = pb_ostream_from_buffer(buffer, BaseResponse_size);

  if (!pb_encode(&output, BaseResponse_fields, &response))
  {
      return 1;
  }

  chnWriteTimeout(chn, buffer, sizeof(buffer), MS2ST(50));

  return 0;
}
