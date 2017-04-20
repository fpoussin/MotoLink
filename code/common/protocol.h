#ifndef PROTOCOL_H
#define PROTOCOL_H

#define VERSION_PROTOCOL 1

#define MAGIC1 (uint8_t)0xAF
#define MAGIC2 (uint8_t)0xEB

/* First 3 bits */
#define MASK_CMD (uint8_t)0x80
#define MASK_REPLY_OK (uint8_t)0x60
#define MASK_DECODE_ERR (uint8_t)0x40
#define MASK_CMD_ERR (uint8_t)0x20
#define MASK_CMD_PART (uint8_t)0x1F

/* Last 5 bits, up to 0x1F */
#define CMD_ERASE (uint8_t)0x01
#define CMD_READ (uint8_t)0x02
#define CMD_WRITE (uint8_t)0x03
#define CMD_RESET (uint8_t)0x04
#define CMD_GET_FLAGS (uint8_t)0x05
#define CMD_WAKE (uint8_t)0x06
#define CMD_BOOT (uint8_t)0x07
#define CMD_GET_MODE (uint8_t)0x08
#define CMD_GET_VERSION (uint8_t)0x09
#define CMD_GET_SIZE (uint8_t)0x0A
#define CMD_GET_SENSORS (uint8_t)0x0B
#define CMD_GET_MONITOR (uint8_t)0x0C
#define CMD_GET_FFT (uint8_t)0x0D
#define CMD_GET_SETTINGS (uint8_t)0x0F
#define CMD_SET_SETTINGS (uint8_t)0x10
#define CMD_GET_TABLES (uint8_t)0x11
#define CMD_GET_TABLES_HEADERS (uint8_t)0x12
#define CMD_SET_TABLES_HEADERS (uint8_t)0x13
#define CMD_CLEAR_CELL (uint8_t)0x14
#define CMD_CLEAR_TABLES (uint8_t)0x15
#define CMD_GET_SERIAL_DATA (uint8_t)0x16

#define FLAG_OK (uint8_t)0x01
#define FLAG_IWDRST (uint8_t)0x02
#define FLAG_SFTRST (uint8_t)0x04
#define FLAG_NOAPP (uint8_t)0x08
#define FLAG_WAKE (uint8_t)0x10
#define FLAG_SWITCH (uint8_t)0x20
#define FLAG_PINRST (uint8_t)0x40

#define MODE_BL (uint8_t)0x01
#define MODE_APP (uint8_t)0x02

#define DATA_BUF_SIZE 256
#define FFT_SIZE 512
#define FFT_FREQ (117263/2)
#define SPECTRUM_SIZE 256
#define KNOCK_RATIO 4.66f
#define KNOCK_MAX (3.3*KNOCK_RATIO)

#define FUNC_RECORD (uint16_t)0x0001
#define FUNC_OBD    (uint16_t)0x0002

#define AFR_INPUT_DISABLED (uint16_t)0x0000
#define AFR_INPUT_MTS (uint8_t)0x01
#define AFR_INPUT_AN (uint8_t)0x02
#define AFR_INPUT_OBD (uint8_t)0x03

#define SENSORS_INPUT_DIRECT (uint8_t)0x00
#define SENSORS_INPUT_COM (uint8_t)0x01
#define SENSORS_INPUT_OBD_KLINE (uint8_t)0x02
#define SENSORS_INPUT_OBD_CAN (uint8_t)0x03
#define SENSORS_INPUT_YAMAHA_CAN (uint8_t)0x04


typedef uint32_t crc_t;

typedef struct {
  uint8_t magic1;
  uint8_t magic2;
  uint8_t type;
  uint8_t len;
} cmd_header_t;

typedef struct {
  uint8_t row;
  uint8_t col;
} cell_t;

typedef struct {
  uint8_t iname;
  uint8_t type;
  uint16_t value;
} value_t;

typedef struct {
  uint8_t columns[16];
  uint8_t rows[11];
  uint8_t padding;
} table_header_t;

typedef struct {
  uint16_t an1;
  uint16_t an2;
  uint16_t an3;
  uint16_t freq1;
  uint16_t freq2;
  uint8_t knock_value;
  uint8_t knock_freq;
  uint16_t rpm;
  uint8_t spd;
  uint8_t afr;
  uint8_t afr_status;
  uint8_t tps;
  uint8_t reserved1;
  uint8_t reserved2;
  cell_t cell;
} sensors_t;

typedef struct {
  table_header_t tableHeaders;
  uint16_t knockFreq;
  uint16_t knockRatio;
  uint16_t tpsMinV;
  uint16_t tpsMaxV;
  uint16_t fuelMinTh;
  uint16_t fuelMaxChange;
  uint16_t AfrMinVal;
  uint16_t AfrMaxVal;
  uint16_t AfrOffset;
  uint16_t functions;
  uint8_t sensorsInput;
  uint8_t afrInput;
  float rpmMult;
  float spdMult;
} settings_t;

typedef struct {
    uint8_t protocol;
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} version_t;

#endif
