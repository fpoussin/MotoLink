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
  100000,    /* 100kHz PWM clock frequency.   */
  500,       /* Initial PWM period 50mS.       */
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

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadBlinker, 64);
static msg_t ThreadBlinker(void *arg) {

  (void)arg;
  systime_t time;
  chRegSetThreadName("blinker");

  while (TRUE) {
    const uint32_t maxDuty = 8000;
    const uint32_t minDuty = 0;
    uint32_t duty = maxDuty;

    for (duty = maxDuty; duty > minDuty; duty-=100)
    {
        time = USBD1.state == USB_ACTIVE ? 7 : 15;
        pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, duty));
        chThdSleepMilliseconds(time);
    }
    for (duty = minDuty; duty < maxDuty; duty +=100)
    {
        time = USBD1.state == USB_ACTIVE ? 7 : 15;
        pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, duty));
        chThdSleepMilliseconds(time);
    }
  }
  return 0;
}

/*
 * USB Bulk thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadUsb, 4096);
static msg_t ThreadUsb(void *arg) {

  uint8_t clear_buff[64];
  uint32_t duty;
  EventListener el1;
  flagsmask_t flags;
  (void)arg;
  chRegSetThreadName("USB");

  bduObjectInit(&BDU1);

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

    duty = 0;
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
        if (duty == 0)
          duty = 7000;
        else
          duty = 0;

        pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, duty));

        read_cmd((BaseChannel *)&BDU1, reset_flags);
        chnReadTimeout((BaseChannel *)&BDU1, clear_buff, 64, MS2ST(25) );
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
