#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "common.h"
#include "usb_config.h"

/* From linker script */
extern uint32_t __user_flash_address__;
extern uint32_t __user_flash_length__;
extern uint32_t __flash_page_size__;

#define USER_APP_ADDR ((uint32_t)&__user_flash_address__)
#define USER_APP_RESET_ADDR (USER_APP_ADDR+281)
#define FLASH_PAGE_SIZE ((uint32_t)&__flash_page_size__)

#define mmio64(x)   (*(volatile uint64_t *)(x))
#define mmio32(x)   (*(volatile uint32_t *)(x))
#define mmio16(x)   (*(volatile uint16_t *)(x))
#define mmio8(x)   (*(volatile uint8_t *)(x))

void startUserApp(void);
void jumpToUser(uint32_t address);
uint8_t checkUserCode(uint32_t address);

uint8_t eraseFlash(uint32_t len);
uint8_t writeFlash(uint32_t addr, uint32_t *buf, uint8_t len);

extern uint8_t bl_wake;
//extern uint8_t reset_flags;

#endif
