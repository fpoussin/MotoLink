/*
 * STM32F303xC memory setup.
 */
 MEMORY
{
    /* Memory regions we use */
    bflash  : org = 0x08000000, len = 64k
    flash   : org = 0x08010000, len = 192k
    ram0    : org = 0x20000000, len = 40k
    ram4    : org = 0x10000000, len = 8k

    /* Default memory regions */
    flash0  : org = 0x08000000, len = 256k
    flash1  : org = 0x00000000, len = 0
    flash2  : org = 0x00000000, len = 0
    flash3  : org = 0x00000000, len = 0
    flash4  : org = 0x00000000, len = 0
    flash5  : org = 0x00000000, len = 0
    flash6  : org = 0x00000000, len = 0
    flash7  : org = 0x00000000, len = 0
    ram1    : org = 0x00000000, len = 0
    ram2    : org = 0x00000000, len = 0
    ram3    : org = 0x00000000, len = 0
    ram5    : org = 0x00000000, len = 0
    ram6    : org = 0x00000000, len = 0
    ram7    : org = 0x00000000, len = 0
}

__user_flash_address__ = ORIGIN(flash);
__user_flash_length__   = LENGTH(flash);

__bl_flash_address__ = ORIGIN(bflash);
__bl_flash_length__   = LENGTH(bflash);
