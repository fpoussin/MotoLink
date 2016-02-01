#include "commands.h"
#include "common.h"
#include "usb_config.h"

#define PUT_TIMEOUT MS2ST(25)

/* TODO: make settings load/save (eeprom) function */
settings_t settings;
static uint8_t input_buf[DATA_BUF_SIZE];

CCM_FUNC uint8_t readCommand(BaseChannel *chn)
{
  cmd_header_t header;
  uint16_t data_read;

  if (chnReadTimeout(chn, (uint8_t *)&header, sizeof(header), MS2ST(25)) < sizeof(header))
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+1, PUT_TIMEOUT);
    return 1;
  }

  // Decode header
  if (header.magic1 != MAGIC1 || header.magic2 != MAGIC2 || !(header.type & MASK_CMD) || header.len < sizeof(header)+1)
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+2, PUT_TIMEOUT);
    return 2;
  }

  // Fetch data
  data_read = (uint8_t)chnReadTimeout(chn, input_buf, header.len-sizeof(header), MS2ST(25));
  if (data_read < (header.len-sizeof(header)))
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+3, PUT_TIMEOUT);
    return 3;
  }
  data_read--; // Discard checksum byte from length

  // Fetch, compute and compare checksums
  const uint8_t cs1 = checksum((uint8_t *)&header, sizeof(header)) + checksum(input_buf, data_read);
  const uint8_t cs2 = input_buf[data_read];

  if (cs1 != cs2)
  {
    chnPutTimeout(chn, MASK_DECODE_ERR+4, PUT_TIMEOUT);
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

    case MASK_CMD | CMD_GET_MONITOR:
      status = sendMonitoring(chn);
      break;

    case MASK_CMD | CMD_GET_FFT:
      status = sendFFT(chn);
      break;

    case MASK_CMD | CMD_SET_SETTINGS:
      status = writeSettings(chn, input_buf, data_read);
      break;

    case MASK_CMD | CMD_GET_SETTINGS:
      status = readSettings(chn);
      break;

    case MASK_CMD | CMD_GET_TABLES:
      status = readTables(chn);
      break;

    case MASK_CMD | CMD_GET_TABLES_HEADERS:
      status = readHeaders(chn);
      break;

    case MASK_CMD | CMD_SET_TABLES_HEADERS:
      status = writeHeaders(chn, input_buf, data_read);
      break;

    case MASK_CMD | CMD_CLEAR_CELL:
      status = clearCell(chn, input_buf, data_read);
      break;

    case MASK_CMD | CMD_CLEAR_TABLES:
      status = clearTables(chn);
      break;

    case MASK_CMD | CMD_WAKE:
      status = wakeHandler(chn);
      break;

    default:
      chnPutTimeout(chn, MASK_DECODE_ERR+5, PUT_TIMEOUT);
      break;
  }

  return status;
}

uint8_t sendMode(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_MODE, PUT_TIMEOUT);
  chnPutTimeout(chn, MODE_APP, PUT_TIMEOUT);
  return 0;
}

uint8_t sendSensors(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_SENSORS, PUT_TIMEOUT);
  chnWriteTimeout(chn, (uint8_t*)&sensors_data, sizeof(sensors_t), PUT_TIMEOUT);
  return 0;
}

uint8_t resetHandler(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_RESET, PUT_TIMEOUT);

  chThdSleepMilliseconds(100);
  chSysDisable();

  NVIC_SystemReset();

  return 0;
}

uint8_t wakeHandler(BaseChannel * chn) {

  /* Dummy command to check connection */
  chnPutTimeout(chn, MASK_REPLY_OK | CMD_WAKE, PUT_TIMEOUT);
  return 0;
}

uint8_t sendMonitoring(BaseChannel * chn) {

  // TODO: Dynamic list with names

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_MONITOR, PUT_TIMEOUT);
  chnWriteTimeout(chn, (uint8_t*)&monitoring, sizeof(monitor_t), PUT_TIMEOUT);
  return 0;
}

uint8_t sendFFT(BaseChannel * chn) {

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_FFT, PUT_TIMEOUT);
  chnWriteTimeout(chn, (uint8_t*)output_knock, sizeof(output_knock), PUT_TIMEOUT*2);
  return 0;
}

uint8_t writeSettings(BaseChannel * chn, uint8_t * buf, uint16_t len)
{
  if (len != sizeof(settings_t))
  {
    chnPutTimeout(chn, MASK_CMD_ERR | CMD_SET_SETTINGS, PUT_TIMEOUT);
    return 1;
  }
  settings_t* settings_buf = (settings_t*)buf;

  memcpy(&settings, settings_buf, sizeof(settings));

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_SET_SETTINGS, PUT_TIMEOUT);
  return 0;
}

uint8_t readSettings(BaseChannel * chn)
{
  chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_SETTINGS, PUT_TIMEOUT);
  chnWriteTimeout(chn, (uint8_t*)&settings, sizeof(settings_t), PUT_TIMEOUT);
  return 0;
}

uint8_t writeHeaders(BaseChannel * chn, uint8_t * buf, uint16_t len)
{
  if (len != sizeof(tableRows)+sizeof(tableColumns))
  {
    chnPutTimeout(chn, MASK_CMD_ERR | CMD_SET_TABLES_HEADERS, PUT_TIMEOUT);
    return 1;
  }

  uint8_t* rows = buf;
  uint8_t* columns = (buf+sizeof(tableRows));

  memcpy(tableRows, rows, sizeof(tableRows));
  memcpy(tableColumns, columns, sizeof(tableColumns));

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_SET_TABLES_HEADERS, PUT_TIMEOUT);
  return 0;
}

uint8_t readHeaders(BaseChannel * chn)
{
  chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_TABLES_HEADERS, PUT_TIMEOUT);
  chnWriteTimeout(chn, tableColumns, sizeof(tableColumns), PUT_TIMEOUT);
  chnWriteTimeout(chn, tableRows, sizeof(tableRows), PUT_TIMEOUT);
  return 0;
}

uint8_t readTables(BaseChannel * chn)
{
  chnPutTimeout(chn, MASK_REPLY_OK | CMD_GET_TABLES, PUT_TIMEOUT);
  chnWriteTimeout(chn, (uint8_t*)tableAFR, sizeof(tableAFR), PUT_TIMEOUT);
  chnWriteTimeout(chn, (uint8_t*)tableKnock, sizeof(tableKnock), PUT_TIMEOUT);
  return 0;
}

uint8_t clearCell(BaseChannel * chn, uint8_t * buf, uint16_t len)
{
  if (len != 3)
  {
    chnPutTimeout(chn, MASK_CMD_ERR | CMD_CLEAR_CELL, PUT_TIMEOUT);
    return 1;
  }

  uint8_t table = *buf;
  cell_t *cell = (cell_t*)(buf+1);

  if (cell->row >= sizeof(tableRows) || cell->col >= sizeof(tableColumns))
  {
    chnPutTimeout(chn, MASK_CMD_ERR | CMD_CLEAR_CELL, PUT_TIMEOUT);
    return 2;
  }

  if (table == 1)
  {
    tableAFR[cell->row][cell->col] = 0;
  }
  else if (table == 2)
  {
    tableKnock[cell->row][cell->col] = 0;
  }

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_CLEAR_CELL, PUT_TIMEOUT);
  return 0;
}

uint8_t clearTables(BaseChannel * chn)
{
  memset(tableAFR, 0, sizeof(tableAFR));
  memset(tableKnock, 0, sizeof(tableKnock));

  chnPutTimeout(chn, MASK_REPLY_OK | CMD_CLEAR_TABLES, PUT_TIMEOUT);
  return 0;
}
