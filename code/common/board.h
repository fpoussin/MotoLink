/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for Motolink board.
 */

/*
 * Board identifier.
 */
#define BOARD_MOTOLINK_REV_A
#define BOARD_NAME                  "Motolink board Revision A"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0
#endif

#define STM32_LSEDRV                (3 << 3)

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                8000000
#endif

//#define STM32_HSE_BYPASS

/*
 * MCU type as defined in the ST header.
 */
#define STM32F303xC
#define STM32F30X //Fix 

/*
 * IO pins assignments.
 */
 
#define SWITCH_PORT GPIOF
#define SWITCH_PAD 4

#define SCS_PORT GPIOB
#define SCS_PAD 0

#define KL_CS_PORT GPIOC
#define KL_CS_PAD 12

#define KLINE_PORT GPIOC
#define KLINE_TX 10
#define KLINE_RX 11
 
#define USB_CONN_PORT GPIOC
#define USB_CONN_PAD   8

#define USB_DETECT_PORT GPIOC
#define USB_DETECT_PAD   15

#define LED_PORT GPIOB
#define LED_RED_PAD 6
#define LED_BLUE_PAD 7

#define LED_TIM TIM4
#define LED_CHN_RED 0
#define LED_CHN_BLUE 1

#define FREQIN_PORT GPIOC
#define FREQIN_PAD1 6
#define FREQIN_PAD2 7

#define AN_PORT GPIOC
#define AN1_PAD 1
#define AN2_PAD 2
#define AN3_PAD 3

#define KNOCK_PORT GPIOB
#define KNOCK_PAD 1

#define DAC_PORT GPIOA
#define DAC_PAD 4

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

#include "board_gpio.h"

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
