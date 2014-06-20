#include "eeprom.h"
#include <string.h>

uint32_t * eeFindCurrentPage(void)
{
	uint32_t addr;
	const uint32_t from = EE_START;
	const uint32_t to = from+EE_LENGTH;

	for (addr=from; addr <= to; addr+=EE_PAGE_SIZE)
	{
		if ((*(uint32_t*)addr) != EE_MAGIC)
		{
			break;
		}
	}
	return (uint32_t*)addr;
}

uint32_t * eeFindNextPage(void)
{
	uint32_t * addr = eeFindCurrentPage();

	/* Check if at last page */
	if((uint32_t)addr == EE_START+EE_LENGTH-EE_PAGE_SIZE)
	{
		addr = (uint32_t*)EE_START;
	}

	return addr;
}

uint8_t eeErasePage(const uint32_t addr)
{
	if (FLASH_ErasePage(addr) != FLASH_COMPLETE) {

	  FLASH_Lock();
	  return 1;
	}
	return 0;
}

uint8_t eePushPage(uint32_t *buffer, uint32_t len)
{
	uint32_t curPage = (uint32_t)eeFindCurrentPage();
	uint32_t nextPage = (uint32_t)eeFindNextPage();
	const uint32_t magic = EE_MAGIC;

	if (nextPage != 0)
	{
		if (eeWriteFlash(nextPage, &magic, 4)) /* Write magic header first */
			return 2;
		if (eeWriteFlash(nextPage+4, buffer, len)) /* Write actual data */
			return 3;
		if (eeErasePage(curPage)) /* Erase previous page */
			return 4;

		return 0;
	}
	return 1;
}

uint8_t eePullPage(uint32_t *buffer, uint32_t len)
{
	const uint32_t* curPage = eeFindCurrentPage();

	if (curPage != NULL)
	{
		memcpy((void*)buffer, (void*)curPage, len);
		return 0;
	}
	return 1;
}

uint8_t eeEraseAll(void)
{
	uint32_t addr;
	const uint32_t from = EE_START;
	const uint32_t to = from+EE_LENGTH;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

	for (addr=from; addr <= to; addr+=EE_PAGE_SIZE) {

		if (FLASH_ErasePage(addr) != FLASH_COMPLETE) {

		  FLASH_Lock();
		  return 1;
		}
	}

	FLASH_Lock();
	return 0;
}

uint8_t eeWriteFlash(const uint32_t addr, const uint32_t *buf, uint8_t len) {

  uint8_t i;

  if (addr > EE_START+EE_LENGTH
		  || addr < EE_START
		  || buf == NULL
		  || !len) {

    return 1;
  }

  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

  for (i=0; i < len; i++) {

    if (FLASH_ProgramWord(addr+(i*4), buf[i]) != FLASH_COMPLETE) {

      FLASH_Lock();
      return 2;
    }
  }

  FLASH_Lock();
  return 0;
}
