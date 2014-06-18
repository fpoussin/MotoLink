#include "communication.h"
#include "common.h"

uint8_t readCommand(BaseChannel *chn, uint8_t flags)
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
    chnPutTimeout(chn, err_buf+4, MS2ST(50));
    return 4;
  }

  // Process command
  uint8_t status = 0;
  switch (header.type)
  {
    case MASK_CMD | CMD_ERASE:
      status = eraseHandler(chn, data_buf);
      break;

    case MASK_CMD | CMD_READ:
      status = readHandler(chn, data_buf);
      break;

    case MASK_CMD | CMD_WRITE:
      status = writeHandler(chn, data_buf, header.len-5);
      break;

    case MASK_CMD | CMD_RESET:
      status = resetHandler(chn);
      break;

    case MASK_CMD | CMD_GET_FLAGS:
      status = sendFlags(chn, flags);
      break;

    case MASK_CMD | CMD_GET_MODE:
      status = sendMode(chn);
      break;

    case MASK_CMD | CMD_WAKE:
      status = wakeHandler(chn);
      break;

    case MASK_CMD | CMD_BOOT:
      status = bootHandler(chn);
      break;

    default:
      chnPutTimeout(chn, err_buf+5, MS2ST(50));
      break;
  }

  if (status) {
    //chnPutTimeout(chn, err_buf, MS2ST(25));
    status += 5;
  }

  return status;
}


uint8_t writeHandler(BaseChannel *chn, uint8_t* buf, uint8_t len) {

  if ((len-4) % 4) {
    chnPutTimeout(chn, MASK_REPLY_ERR | CMD_WRITE, MS2ST(50));
    return 1;
  }

  uint32_t offset = leToInt(buf);
  uint32_t *data_buf = (uint32_t*)(buf+4);
  uint8_t replbuf = MASK_REPLY_OK | CMD_WRITE;

  /* Deduct buffer space used by address */
  uint8_t res = writeFlash(offset, data_buf, (len-4)/4);

  if (res != 0) replbuf = MASK_REPLY_ERR | CMD_WRITE;
  chnPutTimeout(chn, replbuf, MS2ST(25));

  return res;
}

uint8_t readHandler(BaseChannel *chn, uint8_t* buf) {

  uint32_t address = leToInt(buf);
  uint32_t buf_len = leToInt(buf+4);

  chnPutTimeout(chn, MASK_REPLY_OK  | CMD_READ, MS2ST(25));
  chnWriteTimeout(chn, (uint8_t*)(address+USER_APP_ADDR), buf_len, MS2ST(50));
  return 0;
}

uint8_t sendFlags(BaseChannel * chn, uint8_t flags) {

  uint8_t buf[2];
  buf[0] = MASK_REPLY_OK  | CMD_GET_FLAGS;
  buf[1] = flags;

  chnWriteTimeout(chn, buf, 2, MS2ST(50));
  return 0;
}

uint8_t sendMode(BaseChannel * chn) {

  uint8_t buf[2];
  buf[0] = MASK_REPLY_OK | CMD_GET_MODE;
  buf[1] = MODE_BL;

  chnWriteTimeout(chn, buf, 2, MS2ST(50));
  return 0;
}

uint8_t eraseHandler(BaseChannel * chn, uint8_t* buf) {

  uint32_t len = leToInt(buf);

  if (eraseFlash(len)) {
    chnPutTimeout(chn, MASK_REPLY_ERR | CMD_ERASE, MS2ST(50));
    return 1;
  }

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_ERASE, MS2ST(50));
  return 0;
}

uint8_t resetHandler(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_RESET, MS2ST(50));

  chThdSleepMilliseconds(500);

  usbDisconnectBus(&USBD1);
  usbStop(&USBD1);

  chSysDisable();

  NVIC_SystemReset();

  return 0;
}

uint8_t wakeHandler(BaseChannel * chn) {

  bl_wake = 1;
  chnPutTimeout(chn, MASK_REPLY_OK | CMD_WAKE, MS2ST(50));
  return 0;
}

uint8_t bootHandler(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_BOOT, MS2ST(50));
  chThdSleepMilliseconds(100);
  usbStop(&USBD1);

  startUserApp();
  return 0;
}
