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
#include "commands.h"
#include "sensors.h"


/*===========================================================================*/
/* Thread pointers.                                                          */
/*===========================================================================*/

Thread* pThreadBDU = NULL;
Thread* pThreadSDU = NULL;
Thread* pThreadSensors = NULL;
Thread* pThreadKnock = NULL;
Thread* pThreadCAN = NULL;

monitor_t monitoring = {0,0,0,0,0,0,0,100};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

void iwdgGptCb(GPTDriver *gptp)
{
  (void) gptp;
  systime_t time = chTimeNow();
  IWDG->KR = ((uint16_t)0xAAAA);
  time = time - chTimeNow();
  monitoring.irq += time;
  chThdSelf()->runtime -= time;
}

GPTConfig gpt1Cfg =
{
  100000,      /* timer clock.*/
  iwdgGptCb,  /* Timer callback.*/
  0
};

/*
 * DAC config
 */
static const DACConfig daccfg1 = {
  DAC_DHRM_12BIT_RIGHT, /* data holding register mode */
  0 /* DAC CR flags */
};

const SerialConfig uartCfg =
{
 10400, // bit rate
 0,
 USART_CR2_STOP1_BITS,
 USART_CR3_HDSEL
};


PWMConfig pwmcfg = {
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

WORKING_AREA(waThreadCAN, 256);
msg_t ThreadCAN(void *p)
{
  EventListener el;
  CANRxFrame rxmsg;

  (void)p;
  chRegSetThreadName("CAN Bus");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);

  while(!chThdShouldTerminate()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE) == 0) {
      chThdSleepMilliseconds(2);
      continue;
    }
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == RDY_OK) {
      /* Process message.*/
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
  return 0;
}

/*
 * USB Bulk thread.
 */
WORKING_AREA(waThreadBDU, 512);
msg_t ThreadBDU(void *arg)
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
    flags = chEvtGetAndClearFlags(&el1);

    idle_duty = usbConnected() ? 500 : 0;

    pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, idle_duty));

    if (flags & CHN_INPUT_AVAILABLE)
    {
      pwmEnableChannel(&PWMD2, LED_BLUE_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));
      readCommand((BaseChannel *)&BDU1);
    }
    else
     chThdSleepMilliseconds(2);
  }
  return 0;
}

/*
 * USB Serial thread.
 */
WORKING_AREA(waThreadSDU, 1024);
msg_t ThreadSDU(void *arg)
{
  uint8_t ubuffer[SERIAL_BUFFERS_SIZE];
  uint8_t sbuffer[SERIAL_BUFFERS_SIZE];
  EventListener el1, el2;
  flagsmask_t flags_usb, flags_uart;
  size_t read, available;
  (void)arg;
  chRegSetThreadName("SDU");
  chEvtRegisterMask(chnGetEventSource(&SDU1), &el1, CHN_INPUT_AVAILABLE);
  chEvtRegisterMask(chnGetEventSource(&SD1), &el2, CHN_INPUT_AVAILABLE);

  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);
  while(SD1.state != SD_READY) chThdSleepMilliseconds(10);

  /* TODO: Unstable? */
  while (TRUE) {

    chEvtWaitAnyTimeout(EVENT_MASK(1), TIME_IMMEDIATE);
    flags_usb = chEvtGetAndClearFlags(&el1);

    if (flags_usb & CHN_INPUT_AVAILABLE) { /* Incoming data from USB */

      available = chQSpaceI(&SDU1.iqueue);
      if (available > sizeof(ubuffer)) available = sizeof(ubuffer);

      pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 0500));
      read = chnReadTimeout((BaseChannel *)&SDU1, ubuffer, available, MS2ST(1));
      chnWriteTimeout((BaseChannel *)&SD1, ubuffer, read, MS2ST(10));
    }
    else
      chThdSleepMilliseconds(1);

    flags_uart = chEvtGetAndClearFlags(&el2);
    if (flags_uart & CHN_INPUT_AVAILABLE) { /* Incoming data from UART */

      available = chQSpaceI(&SD1.iqueue);
      if (available > sizeof(sbuffer)) available = sizeof(sbuffer);

      /* Read seems incorrect */
      pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 0500));
      read = chnReadTimeout((BaseChannel *)&SD1, sbuffer, available, MS2ST(1));
      chnWriteTimeout((BaseChannel *)&SDU1, sbuffer, read, MS2ST(10));
    }
    else
      chThdSleepMilliseconds(1);

    pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));
  }
  return 0;
}

/*
 * Sensors thread.
 */
WORKING_AREA(waThreadSensors, 32);
msg_t ThreadSensors(void *arg)
{
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
 * Knock processing thread.
 */
WORKING_AREA(waThreadKnock, 128);
msg_t ThreadKnock(void *arg)
{
  (void)arg;
  chRegSetThreadName("Knock");

  q15_t maxValue = 0;
  uint32_t maxIndex = 0;

  /* Initialize the CFFT/CIFFT module */
  arm_cfft_radix4_instance_q15 S;
  arm_cfft_radix4_init_q15(&S, FFT_SIZE, 0, 1);

  while (TRUE)
  {
    while (!knockDataReady) chThdSleepMilliseconds(2);
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
 * CPU Load Monitoring thread.
 */
WORKING_AREA(waThreadMonitor, 256);
msg_t ThreadMonitor(void *arg)
{
  (void)arg;
  chRegSetThreadName("Monitor");
  uint32_t thTicks[7];
  uint32_t runtime_total;
  Thread* pThreadMonitor = chThdSelf();

  while (TRUE)
  {
	chSysLock();

	pThreadBDU->runtime = 0;
	pThreadSDU->runtime = 0;
	pThreadCAN->runtime = 0;
	pThreadKnock->runtime = 0;
	pThreadSensors->runtime = 0;
	pThreadMonitor->runtime = 0;
	chSysGetIdleThread()->runtime = 0;
	irqtotal = 0;

	chSysUnlock();

	/* Populate load data */
	chThdSleepMilliseconds(500);

	chSysLock();

	thTicks[0] = pThreadBDU->runtime;
	thTicks[1] = pThreadSDU->runtime;
	thTicks[2] = pThreadCAN->runtime;
	thTicks[3] = pThreadKnock->runtime;
	thTicks[4] = pThreadSensors->runtime;
	thTicks[5] = pThreadMonitor->runtime;
	thTicks[6] = chSysGetIdleThread()->runtime;

	runtime_total = pThreadBDU->runtime
	    +pThreadSDU->runtime
	    +pThreadCAN->runtime
	    +pThreadKnock->runtime
	    +pThreadSensors->runtime
	    +pThreadMonitor->runtime
	    +chSysGetIdleThread()->runtime;

	monitoring.bdu = (thTicks[0]*10000)/runtime_total;
	monitoring.sdu = (thTicks[1]*10000)/runtime_total;
	monitoring.can = (thTicks[2]*10000)/runtime_total;
	monitoring.knock = (thTicks[3]*10000)/runtime_total;
	monitoring.sensors = (thTicks[4]*10000)/runtime_total;
	monitoring.monitor = (thTicks[5]*10000)/runtime_total;
	monitoring.irq = ((float)irqtotal/0.72f)/(float)runtime_total; /* From 72Mhz cycle counter, == ((n*10000)/72000) */
	monitoring.idle = ((thTicks[6]*10000)/runtime_total) - monitoring.irq;

	chSysUnlock();
  }
  return 0;
}


/*
 * Application entry point.
 */
int main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  driversInit();
  chSysInit();

  usbDisconnectBus(serusbcfg.usbp);

  gptStart(&GPTD1, &gpt1Cfg);
  gptStartContinuous(&GPTD1, 20000);

  pwmStart(&PWMD2, &pwmcfg);
  pwmEnableChannel(&PWMD2, LED_GREEN_PAD, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 8000));

  /*
   * Initialize extra driver objects.
   */
  sduObjectInit(&SDU1);
  bduObjectInit(&BDU1);
  tmObjectInit(&irqtime);

  /*
   * Start peripherals
   */
  RCC->CFGR2 &= ~RCC_CFGR2_ADCPRE12 | ~RCC_CFGR2_ADCPRE34; // Erase registers
  RCC->CFGR2 |= RCC_CFGR2_ADCPRE12_DIV128 | RCC_CFGR2_ADCPRE34_DIV32; // Set prescalers
  sdStart(&SD1, &uartCfg);
  usbStart(serusbcfg.usbp, &usbcfg);
  sduStart(&SDU1, &serusbcfg);
  bduStart(&BDU1, &bulkusbcfg);
  canStart(&CAND1, &cancfg);
  dacStart(&DACD1, &daccfg1);
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD3, NULL);
  timcapStart(&TIMCAPD3, &tc_conf);

  /* ADC 3 Ch1 Offset. -2048 */
  ADC3->OFR1 = ADC_OFR1_OFFSET1_EN | ((1 << 26) & ADC_OFR1_OFFSET1_CH) | 0x0800;

  dacConvert(&DACD1, 0x800);
  adcStartConversion(&ADCD1, &adcgrpcfg_sensors, samples_sensors, ADC_GRP1_BUF_DEPTH);
  adcStartConversion(&ADCD3, &adcgrpcfg_knock, samples_knock, ADC_GRP2_BUF_DEPTH);
  timcapEnable(&TIMCAPD3);

  /*
   * Creates the threads.
   */
  pThreadBDU = chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  pThreadSDU = chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO, ThreadSDU, NULL);
  pThreadSensors = chThdCreateStatic(waThreadSensors, sizeof(waThreadSensors), HIGHPRIO, ThreadSensors, NULL);
  pThreadKnock = chThdCreateStatic(waThreadKnock, sizeof(waThreadKnock), NORMALPRIO, ThreadKnock, NULL);
  pThreadCAN = chThdCreateStatic(waThreadCAN, sizeof(waThreadCAN), NORMALPRIO, ThreadCAN, NULL);
  /* Create last as it uses pointers from above */
  chThdCreateStatic(waThreadMonitor, sizeof(waThreadMonitor), NORMALPRIO+1, ThreadMonitor, NULL);

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
