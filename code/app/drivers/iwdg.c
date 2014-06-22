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
 * @file    iwdg.c
 * @brief   IWDG Driver code.
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

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   IWDG Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void iwdgInit(void)
{
  iwdg_lld_init();
}


/**
 * @brief   Configures and activates the IWDG peripheral.
 *
 * @param[in] iwdgp     pointer to the @p IWDGDriver object
 * @param[in] config    pointer to the @p IWDGConfig object
 *
 * @api
 */
void iwdgStart( IWDGDriver *iwdgp, const IWDGConfig *config )
{
  chDbgCheck((iwdgp != NULL) && (config != NULL), "iwdgStart");

  chSysLock();
  iwdg_lld_start( iwdgp, config );
  iwdgp->state = IWDG_READY;
  chSysUnlock();
}

/**
 * @brief   Resets IWDG's counter.
 *
 * @param[in] iwdgp      pointer to the @p IWDGDriver object
 *
 * @api
 */
void iwdgReset( IWDGDriver *iwdgp )
{
  chDbgCheck( iwdgp != NULL, "iwdgReset");

  chSysLock();
  iwdg_lld_reset( iwdgp );
  chSysUnlock();
}

#endif /* HAL_USE_IWDG */

/** @} */
