/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

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

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

/**
 * @file    templates/xxx_lld.c
 * @brief   IWDG Driver subsystem low level driver source template.
 *
 * @addtogroup IWDG
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "iwdg.h"

#if HAL_USE_IWDG || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define KR_KEY_Reload    ((uint16_t)0xAAAA)
#define KR_KEY_Enable    ((uint16_t)0xCCCC)
#define KR_KEY_Write     ((uint16_t)0x5555)
#define KR_KEY_Protect   ((uint16_t)0x0000)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
IWDGDriver IWDGD;
/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level IWDG driver initialization.
 *
 * @notapi
 */
void iwdg_lld_init(void) {
    IWDGD.state  = IWDG_STOP;
    IWDGD.iwdg   = IWDG;
}

/**
 * @brief   Configures and activates the IWDG peripheral.
 *
 * @param[in] iwdgp      pointer to the @p IWDGDriver object
 * @param[in] ms         IWDG reload time in milliseconds.
 *
 * @notapi
 */
void iwdg_lld_start( IWDGDriver *iwdgp, const IWDGConfig *cfg )
{
    // Clock speed is 40'000Hz
    // Max counter value is 2^12-1.
    
    IWDG_TypeDef * d = iwdgp->iwdg;
    d->KR  = KR_KEY_Write;
    d->PR  = cfg->div;
    d->RLR = (cfg->counter <= IWDG_COUNTER_MAX) ? cfg->counter : IWDG_COUNTER_MAX;
    d->KR  = KR_KEY_Reload;
    d->KR  = KR_KEY_Enable;
}

/**
 * @brief   Reloads IWDG's counter.
 *
 * @param[in] idwgp pointer to the @p IWDGDriver object
 *
 * @notapi
 */
void iwdg_lld_reset( IWDGDriver * iwdgp )
{
    iwdgp->iwdg->KR = KR_KEY_Reload;
}

#endif /* HAL_USE_XXX */

/** @} */
