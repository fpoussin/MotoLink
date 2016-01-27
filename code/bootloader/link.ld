/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * STM32F303xC memory setup.
 */
MEMORY
{
    flash  : org = 0x08000000, len = 20k
    eeprom : org = 0x8025000, len = 108k
    ram0   : org = 0x20000000, len = 40k
    ram1   : org = 0x00000000, len = 0
    ram2   : org = 0x00000000, len = 0
    ram3   : org = 0x00000000, len = 0
    ram4   : org = 0x10000000, len = 8k
    ram5   : org = 0x00000000, len = 0
    ram6   : org = 0x00000000, len = 0
    ram7   : org = 0x00000000, len = 0
}
/*
MEMORY
{
    flash (rx) : org = 0x08000000, len = 20k
    user_flash (rx) : org = 0x08005000, len = 128k
    eeprom (rwx) : org = 0x8025000, len = 108k
    ram (rwx) : org = 0x20000000, len = 40k
    ccmram (rwx) : org = 0x10000000, len = 8k
}
*/
__user_flash_address__ = ORIGIN(flash);
__user_flash_length__   = LENGTH(flash);

INCLUDE ../common/link.ld
