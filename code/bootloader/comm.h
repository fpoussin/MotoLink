#include "ch.h"
#include "hal.h"
#include "stm32f30x_flash.h"
#include "usb_config.h"

#define MAGIC 0xAFEB
#define MAGIC1 0xAF
#define MAGIC2 0xEB

#define MASK_CMD 0xA0
#define MASK_REPLY_OK 0x50
#define MASK_REPLY_ERR 0x70

#define CMD_ERASE 0x01
#define CMD_READ 0x02
#define CMD_WRITE 0x03
#define CMD_RESET 0x04
#define CMD_GET_FLAGS 0x05

#define FLAG_OK 0x01
#define FLAG_IWDRST 0x02
#define FLAG_SFTRST 0x03

#define ERASE_OK 0x10

#define DATA_BUF_SIZE 256

typedef struct {
  uint8_t magic1;
  uint8_t magic2;
  uint8_t type;
  uint8_t len;
} cmd_header_t;


uint8_t read_cmd(BaseChannel *chn, uint8_t flags);
uint8_t writeHandler(BaseChannel *chn, uint8_t* buf, uint8_t len);
uint8_t readHandler(BaseChannel *chn, uint8_t* buf);
uint8_t eraseHandler(BaseChannel * chn, uint8_t len);
uint8_t resetHandler(BaseChannel * chn);
uint8_t sendFlags(BaseChannel * chn, uint8_t flags);

