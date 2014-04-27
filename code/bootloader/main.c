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
#include "comm.h"

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

uint8_t reset_flags = 0;

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    systime_t time = USBD1.state == USB_ACTIVE ? 250 : 500;
    palClearPad(LED_PORT, LED_GREEN_PAD);
    chThdSleepMilliseconds(time);
    palSetPad(LED_PORT, LED_GREEN_PAD);
    chThdSleepMilliseconds(time);
  }
}

/*
 * USB Bulk thread, times are in milliseconds.
 */
static WORKING_AREA(waThread2, 8192);
static msg_t Thread2(void *arg) {

  uint8_t clear_buff[64];
  EventListener el1;
  flagsmask_t flags;
  (void)arg;
  chRegSetThreadName("USB");

  /* Wait for USB connection */
  while(!usbDetect()) chThdSleepMilliseconds(10);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(500);
  usbStart(&USBD1, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  /*
   * Initializes a Bulk USB driver.
   */
  bduObjectInit(&BDU1);
  bduStart(&BDU1, &bulkusbcfg);

  chEvtRegisterMask(chnGetEventSource(&BDU1), &el1, ALL_EVENTS);

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(BDU1.state != BDU_READY) chThdSleepMilliseconds(10);

  while (TRUE) {

    chEvtWaitAny(ALL_EVENTS);
    chSysLock();
    flags = chEvtGetAndClearFlagsI(&el1);
    chSysUnlock();

    if (flags & CHN_INPUT_AVAILABLE) {

      uint8_t cmd_res = read_cmd((BaseChannel *)&BDU1, reset_flags);

      chnReadTimeout((BaseChannel *)&BDU1, clear_buff, 64, MS2ST(25) );

    }
  }
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
    palSetPad(LED_PORT, LED_BLUE_PAD);
  }

  /*!< Remove reset flags */
  RCC->CSR |= RCC_CSR_RMVF;

  halInit();
  chSysInit();

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+1, Thread2, NULL);

  /* If USB is plugged, probe it, else boot directly */
  if (usbDetect()) {

    /* Wait half a second for the bootloader GUI to connect */
    chThdSleepMilliseconds(500);
  }

  /* If BL did not get wakeup cmd, no reset flags and user app looks good, launch it */
  if (!bl_wake && reset_flags == FLAG_OK && checkUserCode(USER_APP_ADDR) == 1) {

      chSysDisable();

      usbDisconnectBus(&USBD1);

      /* Setup IWDG in case the target application does not load */

      const uint32_t LsiFreq = 42000;
      IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

      IWDG_SetPrescaler(IWDG_Prescaler_32);

      IWDG_SetReload(LsiFreq/128);

      IWDG_Enable();

      jumpToUser(USER_APP_ADDR);
  }

  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
}
