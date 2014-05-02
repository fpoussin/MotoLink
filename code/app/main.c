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

#include "usb_config.h"

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

/*
 * USB Bulk thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadBDU, 128);
static msg_t ThreadBDU(void *arg) {

  uint8_t bp;
  EventListener el1;
  flagsmask_t flags;
  (void)arg;
  chRegSetThreadName("USB Bulk");
  chEvtRegisterMask(chnGetEventSource(&BDU1), &el1, CHN_INPUT_AVAILABLE);

  while(BDU1.state != BDU_READY) chThdSleepMilliseconds(100);

  while (TRUE) {

    chEvtWaitOneTimeout(EVENT_MASK(1), MS2ST(10));
    flags = chEvtGetAndClearFlags(&el1);

    if (flags & CHN_INPUT_AVAILABLE) {

      chnReadTimeout((BaseChannel *)&BDU1, &bp, 1, MS2ST(10));

      if (bp == 0xAB) {

        chSysLockFromIsr();

        usbDisconnectBus(&USBD1);
        usbStop(&USBD1);

        chSysDisable();

        NVIC_SystemReset();
      }

      bp *=2;
      chnWriteTimeout((BaseChannel *)&BDU1, &bp, 1, MS2ST(10));

    }
  }
}

/*
 * IWDG thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadIWDG, 64);
static msg_t ThreadIWDG(void *arg) {

  (void)arg;
  chRegSetThreadName("IWDG");
  while (TRUE) {
    chThdSleepMilliseconds(10);
    IWDG->KR = ((uint16_t)0xAAAA);
  }
}


/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Initializes a Bulk USB driver.
   */
  bduObjectInit(&BDU1);
  bduStart(&BDU1, &bulkusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  /*
   * Creates the blinker and bulk threads.
   */
  chThdCreateStatic(ThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  chThdCreateStatic(waThreadIWDG, sizeof(waThreadIWDG), NORMALPRIO+1, ThreadIWDG, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
}
