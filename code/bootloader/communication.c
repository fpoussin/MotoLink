#include "communication.h"
#include "common.h"
#include "storage.h"

CCM_FUNC uint8_t readCommand(BaseChannel *chn, uint8_t flags)
{
  cmd_header_t header;
  uint8_t data_buf[DATA_BUF_SIZE];
  uint16_t data_read;

  if (chnReadTimeout(chn, (uint8_t *)&header, sizeof(cmd_header_t), TIME_MS2I(50)) < sizeof(cmd_header_t))
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+1, TIME_MS2I(25));
    return 1;
  }

  // Decode header
  if (header.magic1 != MAGIC1 || header.magic2 != MAGIC2 || !(header.type & MASK_CMD) || header.len < sizeof(header)+1 )
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+2, TIME_MS2I(25));
    return 2;
  }

  // Fetch data
  data_read = (uint8_t)chnReadTimeout(chn, data_buf, header.len-sizeof(header), TIME_MS2I(25));
  if (data_read < (header.len-sizeof(header)))
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+3, TIME_MS2I(25));
    return 3;
  }
  data_read--; // Ignore CS byte

  // Fetch, compute and compare checksums
  const uint8_t cs1 = checksum((uint8_t *)&header, sizeof(header)) + checksum(data_buf, data_read);
  const uint8_t cs2 = data_buf[data_read];

  if (cs1 != cs2)
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+4, TIME_MS2I(25));
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
      status = writeHandler(chn, data_buf, data_read);
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

    case MASK_CMD | CMD_GET_VERSION:
      status = sendVersion(chn);
      break;

    case MASK_CMD | CMD_WAKE:
      status = wakeHandler(chn);
      break;

    case MASK_CMD | CMD_BOOT:
      status = bootHandler(chn);
      break;

    default:
      chnPutTimeout(chn, MASK_DECODE_ERR+5, TIME_MS2I(25));
      break;
  }

  return status;
}


CCM_FUNC uint8_t writeHandler(BaseChannel *chn, uint8_t* buf, uint8_t len) {

  if ((len-4) % 4) {
    chnPutTimeout(chn, MASK_CMD_ERR | CMD_WRITE, TIME_MS2I(25));
    return 1;
  }

  uint32_t offset = leToUInt32(buf);
  uint32_t *data_buf = (uint32_t*)(buf+4);
  uint8_t replbuf = MASK_REPLY_OK | CMD_WRITE;

  /* Deduct buffer space used by address */
  uint8_t res = writeFlash(offset, data_buf, (len-4)/4);

  if (res != 0) replbuf = MASK_CMD_ERR | CMD_WRITE;
  chnPutTimeout(chn, replbuf, TIME_MS2I(25));

  return res;
}

CCM_FUNC uint8_t readHandler(BaseChannel *chn, uint8_t* buf) {

  uint32_t address = leToUInt32(buf);
  uint32_t buf_len = leToUInt32(buf+4);

  chnPutTimeout(chn, MASK_REPLY_OK  | CMD_READ, TIME_MS2I(25));
  chnWriteTimeout(chn, (uint8_t*)(address+USER_APP_ADDR), buf_len, TIME_MS2I(50));
  return 0;
}

CCM_FUNC uint8_t sendFlags(BaseChannel * chn, uint8_t flags) {

  uint8_t buf[2];
  buf[0] = MASK_REPLY_OK  | CMD_GET_FLAGS;
  buf[1] = flags;

  chnWriteTimeout(chn, buf, 2, TIME_MS2I(50));
  return 0;
}

CCM_FUNC uint8_t sendMode(BaseChannel * chn) {

  uint8_t buf[2];
  buf[0] = MASK_REPLY_OK | CMD_GET_MODE;
  buf[1] = MODE_BL;

  chnWriteTimeout(chn, buf, 2, TIME_MS2I(50));
  return 0;
}

CCM_FUNC uint8_t eraseHandler(BaseChannel * chn, uint8_t* buf) {

  uint32_t len = leToUInt32(buf);

  if (eraseFlash(len)) {
    chnPutTimeout(chn, MASK_CMD_ERR | CMD_ERASE, TIME_MS2I(50));
    return 1;
  }

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_ERASE, TIME_MS2I(50));
  return 0;
}

CCM_FUNC uint8_t resetHandler(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_RESET, TIME_MS2I(50));
  chThdSleepMilliseconds(300);

  usbDisconnectBus(&USBD1);
  usbStop(&USBD1);

  chSysDisable();
  NVIC_SystemReset();

  return 0;
}

CCM_FUNC uint8_t wakeHandler(BaseChannel * chn) {

  bl_wake = 1;
  chnPutTimeout(chn, MASK_REPLY_OK | CMD_WAKE, TIME_MS2I(50));
  return 0;
}

CCM_FUNC uint8_t bootHandler(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_BOOT, TIME_MS2I(50));
  chThdSleepMilliseconds(200);

  usbDisconnectBus(&USBD1);
  usbStop(&USBD1);

  startUserApp();

  return 0;
}

CCM_FUNC uint8_t sendVersion(BaseChannel *chn)
{
    chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_VERSION, TIME_MS2I(50));
    chnWriteTimeout(chn, (uint8_t*)versions, sizeof(version_t)*2, TIME_MS2I(50));
    return 0;
}
