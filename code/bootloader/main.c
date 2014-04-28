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
static WORKING_AREA(waThreadBlinker, 128);
static msg_t ThreadBlinker(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    systime_t time = USBD1.state == USB_ACTIVE ? 250 : 500;
    palClearPad(LED_PORT, LED_GREEN_PAD);
    chThdSleepMilliseconds(time);
    palSetPad(LED_PORT, LED_GREEN_PAD);
    chThdSleepMilliseconds(time);
  }
  return 0;
}

/*
 * USB Bulk thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadUsb, 8192);
static msg_t ThreadUsb(void *arg) {

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

  while (USBD1.state != USB_STOP && BDU1.state != BDU_STOP) {

    chEvtWaitAny(ALL_EVENTS);
    chSysLock();
    flags = chEvtGetAndClearFlagsI(&el1);
    chSysUnlock();

    if (flags & CHN_INPUT_AVAILABLE) {

      read_cmd((BaseChannel *)&BDU1, reset_flags);

      chnReadTimeout((BaseChannel *)&BDU1, clear_buff, 64, MS2ST(25) );

    }
  }

  /* USB stopped because we are launching user app */
  while (TRUE) {
    chThdSleepMilliseconds(100);
  };
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
    palSetPad(LED_PORT, LED_BLUE_PAD);
  }

  /*!< Remove reset flags */
  RCC->CSR |= RCC_CSR_RMVF;

  halInit();
  chSysInit();

  chThdCreateStatic(waThreadBlinker, sizeof(waThreadBlinker), NORMALPRIO, ThreadBlinker, NULL);
  chThdCreateStatic(waThreadUsb, sizeof(waThreadUsb), NORMALPRIO+1, ThreadUsb, NULL);

  /* If USB is plugged, probe it, else boot directly */
  if (usbDetect()) {

    /* Wait one second for the bootloader GUI to send wake up command */
    chThdSleepMilliseconds(1000);
  }

  /* If BL did not get wake up command, no reset flags and user app looks good, launch it */
  if (!bl_wake && reset_flags == FLAG_OK && checkUserCode(USER_APP_ADDR) == 1) {

      bduStop(&BDU1);
      usbStop(&USBD1);
      usbDisconnectBus(&USBD1);

      chSysDisable();

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
