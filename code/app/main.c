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

/*
 * USB Bulk thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadBDU, 128);
static msg_t ThreadBDU(void *arg) {

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

    /* Wait for USB connection */
    while(!usbConnected()) chThdSleepMilliseconds(20);

    /*
     * Activates the USB driver and then the USB bus pull-up on D+.
     * Note, a delay is inserted in order to not have to disconnect the cable
     * after a reset.
     */
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

        //read_cmd((BaseChannel *)&BDU1, reset_flags);
        chnReadTimeout((BaseChannel *)&BDU1, clear_buff, sizeof(clear_buff), MS2ST(25) );
        pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 500));
      }
    }
  }
  return 0;
}

/*
 * USB Serial thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadSDU, 256);
static msg_t ThreadSDU(void *arg) {

  uint8_t buffer[SERIAL_BUFFERS_SIZE];
  EventListener el1, el2;
  flagsmask_t flags_usb, flags_uart;
  size_t read, available;
  (void)arg;
  chRegSetThreadName("USB Serial");
  chEvtRegisterMask(chnGetEventSource(&SDU1), &el1, CHN_INPUT_AVAILABLE);
  chEvtRegisterMask(chnGetEventSource(&SD1), &el2, CHN_INPUT_AVAILABLE);

  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);
  while(SD1.state != SD_READY) chThdSleepMilliseconds(10);

  while (TRUE) {

    chEvtWaitOneTimeout(EVENT_MASK(1), MS2ST(10));
    flags_usb = chEvtGetAndClearFlags(&el1);
    flags_uart = chEvtGetAndClearFlags(&el2);

    if (flags_usb & CHN_INPUT_AVAILABLE) { /* Incoming data from USB */

      available = chQSpaceI(&SDU1.iqueue);
      if (available > sizeof(buffer)) available = sizeof(buffer);

      read = chnReadTimeout((BaseChannel *)&SDU1, buffer, available, MS2ST(10));
      chnWriteTimeout((BaseChannel *)&SD1, buffer, read, MS2ST(10));
    }

    if (flags_uart & CHN_INPUT_AVAILABLE) { /* Incoming data from UART */

      available = chQSpaceI(&SD1.iqueue);
      if (available > sizeof(buffer)) available = sizeof(buffer);

      read = chnReadTimeout((BaseChannel *)&SD1, buffer, available, MS2ST(10));
      chnWriteTimeout((BaseChannel *)&SDU1, buffer, read, MS2ST(10));
    }

  }
  return 0;
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
  return 0;
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

  pwmStart(&PWMD2, &pwmcfg);
  pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));

  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Initializes and start a Bulk USB driver.
   */
  bduObjectInit(&BDU1);
  bduStart(&BDU1, &bulkusbcfg);

  /*
   * Starts a Serial driver.
   */
  sdStart(&SD1, &uartCfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  //usbDisconnectBus(serusbcfg.usbp);
  //chThdSleepMilliseconds(500);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  /*
   * Creates the blinker and bulk threads.
   */
  chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO, ThreadSDU, NULL);
  chThdCreateStatic(waThreadIWDG, sizeof(waThreadIWDG), NORMALPRIO+1, ThreadIWDG, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
}
