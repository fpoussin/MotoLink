#include "bootloader.h"

uint8_t bl_wake = 0;

void startIWDG(void) {

    const uint16_t LsiFreq = 40000;
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    IWDG_SetPrescaler(IWDG_Prescaler_32); // 1250

    IWDG_SetReload(LsiFreq/128); // (1/1250)*(40000/128) = 250ms
    IWDG_ReloadCounter();

    IWDG_Enable();

}

void startUserApp(void) {

  usbStop(&USBD1);
  usbDisconnectBus(&USBD1);

  chSysDisable();

  /* Setup IWDG in case the target application does not load */
  startIWDG();

  jumpToUser(USER_APP_ADDR);
}

void jumpToUser(uint32_t address) {

  typedef void (*pFunction)(void);

  pFunction Jump_To_Application;

  /* variable that will be loaded with the start address of the application */
  vu32* JumpAddress;
  const vu32* ApplicationAddress = (vu32*) address;

  /* get jump address from application vector table */
  JumpAddress = (vu32*) ApplicationAddress[1];

  /* load this address into function pointer */
  Jump_To_Application = (pFunction) JumpAddress;

  /* Clear pending interrupts just to be on the safe side*/
  SCB_ICSR = ICSR_PENDSVCLR;

  /* Disable all interrupts */
  uint8_t i;
  for(i=0; i<8; i++)
    NVIC->ICER[i] = NVIC->IABR[i];

  /* set stack pointer as in application's vector table */
  __set_MSP((u32) (ApplicationAddress[0]));
  Jump_To_Application();
}

uint8_t checkUserCode(uint32_t address) {

  const uint32_t sp = *(vu32 *) address;

  if ((sp & 0x2FFF0000) == 0x20000000 // SRAM
      || (sp & 0x1FFF0000) == 0x10000000) // CCM RAM
  {
    return 1;
  }
  return 0;
}

uint8_t eraseFlash(uint32_t len) {

  uint32_t addr;
  const uint32_t from = USER_APP_ADDR;
  const uint32_t to = from+len;

  if (len % 4) {
    return 1;
  }

  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

  for (addr=USER_APP_ADDR; addr <= to; addr+=FLASH_PAGE_SIZE) {

    if (FLASH_ErasePage(addr) != FLASH_COMPLETE) {

      FLASH_Lock();
      return 1;
    }
  }

  FLASH_Lock();
  return 0;
}

uint8_t writeFlash(uint32_t addr, uint32_t *buf, uint8_t len) {

  uint8_t i;

  if (addr > 196*1024 || buf == NULL || !len) {

    return 1;
  }

  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

  for (i=0; i < len; i++) {

    if (FLASH_ProgramWord(USER_APP_ADDR+addr+(i*4), buf[i]) != FLASH_COMPLETE) {

      FLASH_Lock();
      return 1;
    }
  }

  FLASH_Lock();
  return 0;
}
