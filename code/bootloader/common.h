#include "ch.h"
#include "hal.h"
#include "stm32f30x_flash.h"


#define USER_APP_ADDR (uint32_t)0x08004000
#define USER_APP_RESET_ADDR (uint32_t)0x08004281
#define FLASH_PAGE_SIZE 0x800
#define FLASH_PAGE_OFFSET 0x0A

#define mmio64(x)   (*(volatile uint64_t *)(x))
#define mmio32(x)   (*(volatile uint32_t *)(x))
#define mmio16(x)   (*(volatile uint16_t *)(x))
#define mmio8(x)   (*(volatile uint8_t *)(x))

void jumpToUser(uint32_t address);
uint8_t checkUserCode(uint32_t address);

uint8_t eraseFlash(uint32_t len);
uint8_t writeFlash(uint32_t addr, uint32_t *buf, uint8_t len);
inline uint32_t leToInt(uint8_t *ptr);
inline uint32_t beToInt(uint8_t *ptr);
