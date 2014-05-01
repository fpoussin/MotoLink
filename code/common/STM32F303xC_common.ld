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
 * ST32F303xC memory setup.
 */
__main_stack_size__     = 0x0400;
__process_stack_size__  = 0x0400;

MEMORY
{
    boot : org = 0x08000000, len = 16k
    flash : org = 0x08004000, len = 200k
    eeprom : org = 0x8036000, len = 40k
    ram : org = 0x20000000, len = 40k
    ccmram : org = 0x10000000, len = 8k
}

__ram_start__           = ORIGIN(ram);
__ram_size__            = LENGTH(ram);
__ram_end__             = __ram_start__ + __ram_size__;

ENTRY(ResetHandler)

