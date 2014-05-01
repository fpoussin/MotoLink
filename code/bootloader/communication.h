#include "ch.h"
#include "hal.h"
#include "stm32f30x_flash.h"
#include "usb_config.h"

#define MAGIC (uint16_t)0xAFEB
#define MAGIC1 (uint8_t)0xAF
#define MAGIC2 (uint8_t)0xEB

#define MASK_CMD (uint8_t)0xA0
#define MASK_REPLY_OK (uint8_t)0x50
#define MASK_REPLY_ERR (uint8_t)0x70

#define CMD_ERASE (uint8_t)0x01
#define CMD_READ (uint8_t)0x02
#define CMD_WRITE (uint8_t)0x03
#define CMD_RESET (uint8_t)0x04
#define CMD_GET_FLAGS (uint8_t)0x05
#define CMD_WAKE (uint8_t)0x06
#define CMD_BOOT (uint8_t)0x07

#define FLAG_OK (uint8_t)0x01
#define FLAG_IWDRST (uint8_t)0x02
#define FLAG_SFTRST (uint8_t)0x03

#define ERASE_OK (uint8_t)0x10

#define DATA_BUF_SIZE 256

typedef struct {
  uint8_t magic1;
  uint8_t magic2;
  uint8_t type;
  uint8_t len;
} cmd_header_t;

extern uint8_t bl_wake;

uint8_t read_cmd(BaseChannel *chn, uint8_t flags);
uint8_t writeHandler(BaseChannel *chn, uint8_t* buf, uint8_t len);
uint8_t readHandler(BaseChannel *chn, uint8_t* buf);
uint8_t eraseHandler(BaseChannel * chn, uint8_t len);
uint8_t resetHandler(BaseChannel * chn);
uint8_t sendFlags(BaseChannel * chn, uint8_t flags);
uint8_t wakeHandler(BaseChannel * chn);
uint8_t bootHandler(BaseChannel * chn);
