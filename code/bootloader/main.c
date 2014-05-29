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

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "common.h"
#include "usb_config.h"
#include "stm32f30x_iwdg.h"
#include "communication.h"

/*===========================================================================*/
/* Config                                                                    */
/*===========================================================================*/

uint8_t reset_flags = FLAG_OK;

static PWMConfig pwmcfg = {
  10000,    /* 10kHz PWM clock frequency.   */
  50,      /* Initial PWM period 10mS.       */
  NULL,
  {
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0
};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

// Duty
#define D(x) PWM_PERCENTAGE_TO_WIDTH(&PWMD2, x)

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadBlinker, 512);
static msg_t ThreadBlinker(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");

  const uint32_t dimmer[] = {D(0), D(200), D(400), D(600), D(800), D(1000), D(1200), D(1400), D(1600), D(1800), D(2000), D(2200),
                             D(2400), D(2600), D(2800), D(3000),  D(3200), D(3400), D(3600), D(3800), D(4000), D(4200), D(4400),
                             D(4600), D(4800), D(5000), D(5200), D(5400), D(5600), D(5800), D(6000), D(6200), D(6400), D(6600),
                             D(6800), D(7000), D(7200), D(7400), D(7600), D(7800), D(8000), D(7800), D(7600), D(7400), D(7200),
                             D(7000), D(6800), D(6600), D(6400), D(6200), D(6000), D(5800), D(5600), D(5400), D(5200), D(5000),
                             D(4800), D(4600), D(4400), D(4200), D(4000), D(3800), D(3600), D(3400), D(3200), D(3000), D(2800),
                             D(2600), D(2400), D(2200), D(2000), D(1800), D(1600), D(1400), D(1200), D(1000), D(800), D(600),
                             D(400), D(200)};


  TIM2->DIER |= TIM_DIER_UDE; /* Timer Update DMA request */
  if (dmaStreamAllocate(STM32_DMA1_STREAM2, 0, NULL, NULL)) while (1) chThdSleepMilliseconds(20);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM2, &TIM2->CCR3);
  dmaStreamSetMemory0(STM32_DMA1_STREAM2, dimmer);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM2, sizeof(dimmer)/4);
  dmaStreamSetMode(STM32_DMA1_STREAM2, STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD
                   | STM32_DMA_CR_EN | STM32_DMA_CR_CIRC | STM32_DMA_CR_DIR_M2P | DMA_CCR_MINC);

  while (TRUE)
  {
    chThdSleepMilliseconds(100);
    if (usbConnected())
      TIM2->PSC = (STM32_TIMCLK1 / 10000) - 1;
    else
      TIM2->PSC = (STM32_TIMCLK1 / 5000) - 1;
  }
  return 0;
}

/*
 * USB Bulk thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadUsb, 4096);
static msg_t ThreadUsb(void *arg) {

  uint8_t clear_buff[64];
  EventListener el1;
  flagsmask_t flags;
  (void)arg;
  chRegSetThreadName("USB");

  bduObjectInit(&BDU1);
  pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 500));

  while (TRUE)
  {
    usbDisconnectBus(serusbcfg.usbp);
    bduStop(&BDU1);
    usbStop(&USBD1);

    /* Wait for USB connection */
    while(!usbConnected()) chThdSleepMilliseconds(20);

    /*
     * Activates the USB driver and then the USB bus pull-up on D+.
     * Note, a delay is inserted in order to not have to disconnect the cable
     * after a reset.
     */
    usbStart(&USBD1, &usbcfg);
    chThdSleepMilliseconds(100);
    usbConnectBus(serusbcfg.usbp);

    /*
     * Start the Bulk USB driver.
     */
    bduStart(&BDU1, &bulkusbcfg);

    chEvtRegisterMask(chnGetEventSource(&BDU1), &el1, ALL_EVENTS);

    while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
    while(BDU1.state != BDU_READY) chThdSleepMilliseconds(10);

    while (USBD1.state != USB_STOP
        && BDU1.state != BDU_STOP
        && usbConnected())
    {
      chEvtWaitAny(ALL_EVENTS);
      chSysLock();
      flags = chEvtGetAndClearFlagsI(&el1);
      chSysUnlock();

      if (flags & CHN_INPUT_AVAILABLE)
      {
        pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));

        read_cmd((BaseChannel *)&BDU1, reset_flags);
        chnReadTimeout((BaseChannel *)&BDU1, clear_buff, sizeof(clear_buff), MS2ST(25) );
        pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 500));
      }
    }
  }
  return 0;
}

/*
 * Application entry point.
 */
int main(void) {

  /*!< Independent Watchdog reset flag */
  if (RCC->CSR & RCC_CSR_IWDGRSTF) {
    /* User App did not start properly */

    reset_flags |= FLAG_IWDRST;
  }

  /*!< Software Reset flag */
  else if (RCC->CSR & RCC_CSR_SFTRSTF) {
    /* Bootloader called by user app */

    reset_flags |= FLAG_SFTRST;
  }

  /*!< Remove reset flags */
  RCC->CSR |= RCC_CSR_RMVF;

  halInit();
  chSysInit();

  pwmStart(&PWMD2, &pwmcfg);

  chThdCreateStatic(waThreadBlinker, sizeof(waThreadBlinker), NORMALPRIO, ThreadBlinker, NULL);
  chThdCreateStatic(waThreadUsb, sizeof(waThreadUsb), NORMALPRIO+1, ThreadUsb, NULL);

  /* If USB is plugged, probe it, else boot directly */
  if (usbConnected()) {

    /* Wait one second for the bootloader GUI to send wake up command */
    chThdSleepMilliseconds(1000);
  }

  /* If BL did not get wake up command, no reset flags and user app looks good, launch it */
  if (!bl_wake && reset_flags == FLAG_OK && checkUserCode(USER_APP_ADDR) == 1) {

    startUserApp();
  }

  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
}
