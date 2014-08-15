#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAGIC1 (uint8_t)0xAF
#define MAGIC2 (uint8_t)0xEB

/* First 3 bits */
#define MASK_CMD (uint8_t)0x80
#define MASK_REPLY_OK (uint8_t)0x40
#define MASK_REPLY_ERR (uint8_t)0x20

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

#define FLAG_OK (uint8_t)0x01
#define FLAG_IWDRST (uint8_t)0x02
#define FLAG_SFTRST (uint8_t)0x04
#define FLAG_NOAPP (uint8_t)0x08
#define FLAG_WAKE (uint8_t)0x10
#define FLAG_SWITCH (uint8_t)0x20

#define MODE_BL (uint8_t)0x01
#define MODE_APP (uint8_t)0x02

#define DATA_BUF_SIZE 256
#define FFT_SIZE 256
#define FFT_FREQ 112500

typedef struct {
  uint8_t magic1;
  uint8_t magic2;
  uint8_t type;
  uint8_t len;
} cmd_header_t;

typedef struct {
  uint16_t an7;
  uint16_t an8;
  uint16_t an9;
  uint16_t freq1;
  uint16_t freq2;
  uint16_t knock_value;
  uint16_t knock_freq;
  uint16_t reserved1;
} sensors_t;

typedef struct {
  uint16_t bdu;
  uint16_t sdu;
  uint16_t can;
  uint16_t knock;
  uint16_t sensors;
  uint16_t monitor;
  uint16_t irq;
  uint16_t idle;
} monitor_t;

#endif
