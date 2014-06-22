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
 * @file    templates/xxx_lld.h
 * @brief   IWDG Driver subsystem low level driver header template.
 *
 * @addtogroup IWDG
 * @{
 */

#ifndef _IWDG_LLD_H_
#define _IWDG_LLD_H_

#if HAL_USE_IWDG || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define IWDG_COUNTER_MAX ( (1<<12)-1 )
#define IWDG_DIV_4   0
#define IWDG_DIV_8   1
#define IWDG_DIV_16  2
#define IWDG_DIV_32  3
#define IWDG_DIV_64  4
#define IWDG_DIV_128 5
#define IWDG_DIV_256 6
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an IWDG driver.
 */
//typedef struct IWDGDriver IWDGDriver;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
    uint16_t    counter;
    uint8_t     div;
} IWDGConfig;

/**
 * @brief   Structure representing an IWDG driver.
 */
struct IWDGDriver {
  /**
   * @brief Driver state.
   */
  iwdgstate_t                state;
  /* End of the mandatory fields.*/
  IWDG_TypeDef               *iwdg;
};

typedef struct IWDGDriver IWDGDriver;

extern IWDGDriver IWDGD;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void iwdg_lld_init(void);
  void iwdg_lld_start( IWDGDriver *iwdgp, const IWDGConfig *cfg );
  void iwdg_lld_reset( IWDGDriver *iwdgp );
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_IWDG */

#endif /* _IWDG_LLD_H_ */

/** @} */
