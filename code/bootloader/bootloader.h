#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "common.h"

#include "stm32f30x_flash.h"
#include "stm32f30x_iwdg.h"
#include "usb_config.h"

#define USER_APP_ADDR (uint32_t)0x08005000
#define USER_APP_RESET_ADDR (uint32_t)0x08005281
#define FLASH_PAGE_SIZE 0x800

#define mmio64(x)   (*(volatile uint64_t *)(x))
#define mmio32(x)   (*(volatile uint32_t *)(x))
#define mmio16(x)   (*(volatile uint16_t *)(x))
#define mmio8(x)   (*(volatile uint8_t *)(x))

void startUserApp(void);
void jumpToUser(uint32_t address);
uint8_t checkUserCode(uint32_t address);

uint8_t eraseFlash(uint32_t len);
uint8_t writeFlash(uint32_t addr, uint32_t *buf, uint8_t len);

#endif