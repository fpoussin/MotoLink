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
#include "usb_config.h"
#include "communication.h"
#include "sensors.h"

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

void iwdgGptCb(GPTDriver *gptp)
{
  (void) gptp;
  IWDG->KR = ((uint16_t)0xAAAA);
}

GPTConfig gpt1Cfg =
{
  100000,    /* timer clock.*/
  iwdgGptCb,        /* Timer callback.*/
  0
};

const uint16_t dac_buffer[1] = {0x800};

/*
 * DAC config
 */
static const DACConfig daccfg1 = {
  10000, /* Multiply the buffer size to the desired frequency in Hz */
  DAC_DHRM_12BIT_RIGHT, /* data holding register mode */
  0 /* DAC CR flags */
};

/*
 * DAC conversion groups, with callbacks.
 */
static const DACConversionGroup dacconvgrp1 = {
  1, /* Channels */
  NULL, /* End of transfer callback */
  NULL, /* Error callback */
  false /*circular mode */
};

const SerialConfig uartCfg =
{
 10400, // bit rate
 0,
 USART_CR2_STOP1_BITS,
 USART_CR3_HDSEL
};


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
static WORKING_AREA(waThreadBDU, 256);
static msg_t ThreadBDU(void *arg)
{
  EventListener el1;
  flagsmask_t flags;
  (void)arg;
  chRegSetThreadName("BDU");
  uint16_t idle_duty = 0;

  chEvtRegisterMask(chnGetEventSource(&BDU1), &el1, ALL_EVENTS);

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(BDU1.state != BDU_READY) chThdSleepMilliseconds(10);

  while (TRUE)
  {
    chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE);
    chSysLock();
    flags = chEvtGetAndClearFlagsI(&el1);
    chSysUnlock();

    idle_duty = usbConnected() ? 500 : 0;

    pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, idle_duty));

    if (flags & CHN_INPUT_AVAILABLE)
    {
      pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));
      read_cmd((BaseChannel *)&BDU1);
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
  chRegSetThreadName("SDU");
  chEvtRegisterMask(chnGetEventSource(&SDU1), &el1, CHN_INPUT_AVAILABLE);
  chEvtRegisterMask(chnGetEventSource(&SD1), &el2, CHN_INPUT_AVAILABLE);

  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);
  while(SD1.state != SD_READY) chThdSleepMilliseconds(10);

  while (TRUE) {

    chEvtWaitOneTimeout(EVENT_MASK(1), TIME_IMMEDIATE);
    flags_usb = chEvtGetAndClearFlags(&el1);
    flags_uart = chEvtGetAndClearFlags(&el2);

    if (flags_usb & CHN_INPUT_AVAILABLE) { /* Incoming data from USB */

      available = chQSpaceI(&SDU1.iqueue);
      if (available > sizeof(buffer)) available = sizeof(buffer);

      pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 0500));
      read = chnReadTimeout((BaseChannel *)&SDU1, buffer, available, MS2ST(10));
      chnWriteTimeout((BaseChannel *)&SD1, buffer, read, MS2ST(10));
    }

    if (flags_uart & CHN_INPUT_AVAILABLE) { /* Incoming data from UART */

      available = chQSpaceI(&SD1.iqueue);
      if (available > sizeof(buffer)) available = sizeof(buffer);

      pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 0500));
      read = chnReadTimeout((BaseChannel *)&SD1, buffer, available, MS2ST(10));
      chnWriteTimeout((BaseChannel *)&SDU1, buffer, read, MS2ST(10));
    }
    pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));

  }
  return 0;
}

/*
 * Sensors thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadSensors, 32);
static msg_t ThreadSensors(void *arg) {

  (void)arg;
  chRegSetThreadName("Sensors");
  while (TRUE)
  {
    chThdSleepMilliseconds(50);
    TIM3->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;
  }
  return 0;
}

/*
 * Knock processing thread, times are in milliseconds.
 */
static WORKING_AREA(waThreadKnock, 64);
static msg_t ThreadKnock(void *arg) {

  (void)arg;
  chRegSetThreadName("Knock");

  q15_t maxValue = 0;
  uint32_t maxIndex = 0;

  /* Initialize the CFFT/CIFFT module */
  arm_cfft_radix4_instance_q15 S;
  arm_cfft_radix4_init_q15(&S, FFT_SIZE, 0, 1);

  while (TRUE)
  {
    while (!knockDataReady) chThdSleepMilliseconds(5);
    knockDataReady = false;

    /* Process the data through the CFFT/CIFFT module */
    arm_cfft_radix4_q15(&S, data_knock);

    /* Process the data through the Complex Magnitude Module for
    calculating the magnitude at each bin */
    memset(output_knock, 0, sizeof(output_knock));
    arm_cmplx_mag_q15(data_knock, output_knock, FFT_SIZE/2); // We only care about the first half
    arm_max_q15(output_knock, FFT_SIZE/2, &maxValue, &maxIndex); // We only care about the first half

    sensors_data.knock_value = maxValue;
    sensors_data.knock_freq = (SAMPLING_RATE*maxIndex)/FFT_SIZE;
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
  timcapInit();

  usbDisconnectBus(serusbcfg.usbp);

  gptStart(&GPTD1, &gpt1Cfg);
  gptStartContinuous(&GPTD1, 5000);

  pwmStart(&PWMD2, &pwmcfg);
  pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));

  /*
   * Initialize extra drivers.
   */
  sduObjectInit(&SDU1);
  bduObjectInit(&BDU1);
  timcapObjectInit(&TIMCAPD3);

  /*
   * Start peripherals
   */
  sdStart(&SD1, &uartCfg);
  usbStart(serusbcfg.usbp, &usbcfg);
  sduStart(&SDU1, &serusbcfg);
  bduStart(&BDU1, &bulkusbcfg);

  /* Start remaining peripherals */
  dacStart(&DACD1, &daccfg1);

  RCC->CFGR2 &= ~RCC_CFGR2_ADCPRE12 | ~RCC_CFGR2_ADCPRE34; //erase register
  RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV128 | RCC_CFGR2_ADCPRE34_DIV32; // set precalers
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD3, NULL);
  timcapStart(&TIMCAPD3, &tc_conf);

  dacStartConversion(&DACD1, &dacconvgrp1, dac_buffer, 1);
  adcStartConversion(&ADCD1, &adcgrpcfg_sensors, samples_sensors, ADC_GRP1_BUF_DEPTH);
  adcStartConversion(&ADCD3, &adcgrpcfg_knock, samples_knock, ADC_GRP2_BUF_DEPTH);

  /*
   * Creates the threads.
   */
  chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO, ThreadSDU, NULL);
  chThdCreateStatic(waThreadSensors, sizeof(waThreadSensors), NORMALPRIO, ThreadSensors, NULL);
  chThdCreateStatic(waThreadKnock, sizeof(waThreadKnock), NORMALPRIO, ThreadKnock, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE)
  {
    chThdSleepMilliseconds(100);

    if (usbConnected())
    {
      usbConnectBus(serusbcfg.usbp);
    }
    else
    {
      usbDisconnectBus(serusbcfg.usbp);
    }
  }
}
