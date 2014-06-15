#include "communication.h"
#include "common.h"
#include "usb_config.h"

uint8_t read_cmd(BaseChannel *chn)
{
  cmd_header_t header;
  uint8_t data_buf[DATA_BUF_SIZE];
  uint8_t err_buf = MASK_REPLY_ERR;

  if (chnReadTimeout(chn, (uint8_t *)&header, sizeof(cmd_header_t), MS2ST(50)) < sizeof(cmd_header_t))
  {
    chnPutTimeout(chn, err_buf+1, MS2ST(25));
    return 1;
  }

  // Decode header
  if (header.magic1 != MAGIC1 || header.magic2 != MAGIC2 || !(header.type & MASK_CMD) || header.len < 5 )
  {
    chnPutTimeout(chn, err_buf+2, MS2ST(25));
    return 2;
  }

  // Fetch data
  if ((uint8_t)chnReadTimeout(chn, data_buf, header.len-4, MS2ST(25)) < (header.len-4))
  {
    chnPutTimeout(chn, err_buf+3, MS2ST(25));
    return 3;
  }

  // Fetch, compute and compare checksums
  const uint8_t cs1 = checksum((uint8_t *)&header, sizeof(header)) + checksum(data_buf, header.len-5);
  const uint8_t cs2 = data_buf[header.len-5];

  if (cs1 != cs2)
  {
    chnPutTimeout(chn, err_buf+4, MS2ST(25));
    return 4;
  }

  // Process command
  uint8_t status = 0;
  switch (header.type)
  {
    case MASK_CMD | CMD_RESET:
      status = resetHandler(chn);
      break;

    case MASK_CMD | CMD_GET_MODE:
      status = sendMode(chn);
      break;

    case MASK_CMD | CMD_GET_SENSORS:
      status = sendSensors(chn);
      break;

    case MASK_CMD | CMD_WAKE:
      status = wakeHandler(chn);
      break;

    default:
      chnPutTimeout(chn, err_buf+5, MS2ST(25));
      break;
  }

  if (status) {
    //chnPutTimeout(chn, err_buf, MS2ST(25));
    status += 5;
  }

  return status;
}

uint8_t sendMode(BaseChannel * chn) {

  uint8_t buf[2];
  buf[0] = MASK_REPLY_OK | CMD_GET_MODE;
  buf[1] = MODE_APP;

  chnWriteTimeout(chn, buf, 2, MS2ST(25));
  return 0;
}

uint8_t sendSensors(BaseChannel * chn) {

  uint8_t buf[1+sizeof(sensors_t)];
  buf[0] = MASK_REPLY_OK | CMD_GET_SENSORS;
  memcpy(buf+1, &sensors_data, sizeof(sensors_t));

  chnWriteTimeout(chn, buf, sizeof(buf), MS2ST(25));
  return 0;
}

uint8_t resetHandler(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_RESET, MS2ST(25));

  chThdSleepMilliseconds(100);
  chSysDisable();

  NVIC_SystemReset();

  return 0;
}

uint8_t wakeHandler(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_WAKE, MS2ST(25));
  return 0;
}
