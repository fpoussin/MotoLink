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
#include "common.h"
#include "communication.h"
#include "storage.h"

/*===========================================================================*/
/* Config                                                                    */
/*===========================================================================*/

volatile uint8_t reset_flags = FLAG_OK;

static PWMConfig pwmcfg = {
  10000,    /* 10kHz PWM clock frequency.   */
  50,      /* Initial PWM period 5mS.       */
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0
};

// K-Line
SerialConfig uart3Cfg =
{
 19200, // bit rate
 0,
 USART_CR2_STOP1_BITS,
 0
};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

// Duty
#define D(x) PWM_PERCENTAGE_TO_WIDTH(&PWMD4, x*100)

/*
 * Blue LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThreadBlinker, 384);
static THD_FUNCTION(ThreadBlinker, arg)
{
  (void)arg;
  chRegSetThreadName("Blinker");

  const uint16_t dimmer[] = {D(0), D(2), D(4), D(6), D(8), D(10), D(12), D(14), D(16), D(18), D(20), D(22),
                             D(24), D(26), D(28), D(30),  D(32), D(34), D(36), D(38), D(40), D(42), D(44),
                             D(46), D(48), D(50), D(52), D(54), D(56), D(58), D(60), D(62), D(64), D(66),
                             D(68), D(70), D(72), D(74), D(76), D(78), D(80), D(78), D(76), D(74), D(72),
                             D(70), D(68), D(66), D(64), D(62), D(60), D(58), D(56), D(54), D(52), D(50),
                             D(48), D(46), D(44), D(42), D(40), D(38), D(36), D(34), D(32), D(30), D(28),
                             D(26), D(24), D(22), D(20), D(18), D(16), D(14), D(12), D(10), D(8), D(6),
                             D(4), D(2)};

  TIM_LED2->DIER |= TIM_DIER_UDE; /* Timer Update DMA request */
  if (dmaStreamAllocate(STM32_DMA1_STREAM7, 1, NULL, NULL)) chSysHalt("TIM DMA error");
  dmaStreamSetPeripheral(STM32_DMA1_STREAM7, &TIM_LED2->CCR_LED2);
  dmaStreamSetMemory0(STM32_DMA1_STREAM7, dimmer);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM7, sizeof(dimmer)/sizeof(uint16_t));
  dmaStreamSetMode(STM32_DMA1_STREAM7, STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_HWORD
                   | STM32_DMA_CR_EN | STM32_DMA_CR_CIRC | STM32_DMA_CR_DIR_M2P | DMA_CCR_MINC);

  while (TRUE)
  {
    chThdSleepMilliseconds(100);
    if (usbConnected())
      TIM_LED2->PSC = (STM32_TIMCLK1 / 10000) - 1;
    else
      TIM_LED2->PSC = (STM32_TIMCLK1 / 5000) - 1;
  }
  return;
}

/*
 * USB Bulk thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThreadBDU, 1024);
CCM_FUNC static THD_FUNCTION(ThreadBDU, arg)
{
  event_listener_t el1;
  eventmask_t flags;
  (void)arg;
  chRegSetThreadName("BDU");
  uint16_t idle_duty = 0;

  chEvtRegisterMask(chnGetEventSource(&SDU2), &el1, ALL_EVENTS);

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(SDU2.state != SDU_READY) chThdSleepMilliseconds(10);

  while (TRUE)
  {
    chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE);
    flags = chEvtGetAndClearFlags(&el1);

    idle_duty = usbConnected() ? 500 : 0;

    pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, idle_duty));

    if (flags & CHN_INPUT_AVAILABLE)
    {
      pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 10000));
      readCommand((BaseChannel *)&SDU2, reset_flags);
    }

    chThdSleepMilliseconds(1);
  }
  return;
}

/*
 * USB Serial thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThreadSDU, 1024);
static THD_FUNCTION(ThreadSDU, arg)
{
  (void)arg;
  uint8_t buffer[SERIAL_BUFFERS_SIZE/2];
  size_t read;
  chRegSetThreadName("SDU");

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);
  while(SD3.state != SD_READY) chThdSleepMilliseconds(10);

  while (TRUE) {

    while(SD3.state != SD_READY) chThdSleepMilliseconds(10);

    read = chnReadTimeout(&SDU1, buffer, sizeof(buffer), MS2ST(5));
    if (read > 0)
    {
      sdWriteTimeout(&SD3, buffer, read, MS2ST(100));
    }

    read = sdReadTimeout(&SD3, buffer, sizeof(buffer), MS2ST(5));
    if (read > 0)
    {
      chnWriteTimeout(&SDU1, buffer, read, MS2ST(100));
    }

    chThdSleepMilliseconds(1);
  }
  return;
}

/*
 * Application entry point.
 */
int main(void)
{
  /* Begin charging switch cap */
  halInit();
  chSysInit();

  DEBUGEN(printf("Bootloader Started\n"));

  /*!< Independent Watchdog reset flag */
  if (RCC->CSR & RCC_CSR_IWDGRSTF) {
    /* User App did not start properly */

    reset_flags |= FLAG_IWDRST;
    DEBUGEN(printf("FLAG_IWDRST\n"));
  }

  /*!< Software Reset flag */
  if (RCC->CSR & RCC_CSR_SFTRSTF) {
    /* Bootloader called by user app */

    reset_flags |= FLAG_SFTRST;
    DEBUGEN(printf("FLAG_SFTRST\n"));
  }

  if (RCC->CSR & RCC_CSR_PINRSTF) {
    /* Bootloader called by reset pin */

    reset_flags |= FLAG_PINRST;
    DEBUGEN(printf("FLAG_PINRST\n"));
  }

  /* Check user app */
  if (checkUserCode(USER_APP_ADDR) != 1) {

    reset_flags |= FLAG_NOAPP;
    DEBUGEN(printf("FLAG_NOAPP\n"));
  }

  /* Give enough time for the switch's debounce cap to charge
   * Given the 25<->55k pullup, 100nf cap, 0.8v threshold voltage
   * it takes 0.7ms<->1.5ms to reach the High level voltage */
  uint32_t i = 0;
  while (i++ < 20000) // 1.4ms * 3
  {
    asm ("nop");
  }

  /* Check boot switch */
  if (getSwitch1()) {

    reset_flags |= FLAG_SWITCH;
    DEBUGEN(printf("FLAG_SWITCH\n"));
  }

  /* Check debugger if is attached */
  if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
    reset_flags |= FLAG_DBG;
    DEBUGEN(printf("FLAG_DBG\n"));
  }

  /*!< Remove hardware reset flags */
  RCC->CSR |= RCC_CSR_RMVF;

  usbDisconnectBus(serusbcfg1.usbp);

  /* Ignore debug and reset pin (pulled down by debugger) */
  if ((reset_flags & ~(FLAG_DBG | FLAG_PINRST)) == FLAG_OK)
  {
    DEBUGEN(printf("Booting...\n"));
    startUserApp();
    while (1);
  }

  DEBUGEN(printf("DFU Mode\n"));

  pwmStart(&PWMD4, &pwmcfg);
  usbStart(&USBD1, &usbcfg);
  sdStart(&SD3, &uart3Cfg);

  sduObjectInit(&SDU1);
  sduObjectInit(&SDU2);

  sduStart(&SDU1, &serusbcfg1);
  sduStart(&SDU2, &serusbcfg2);

  eeInit();

  // Compare and update versions in EEprom if needed.
  version_t v;
  if (readVersionFromEE(VERSION_IDX_BL, &v) == 0 && memcmp(&versions, &v, sizeof(version_t)) != 0) {
    writeVersionToEE(VERSION_IDX_BL, &versions[VERSION_IDX_BL]);
  }
  if (readVersionFromEE(VERSION_IDX_APP, &v) == 0 ) {
    memcpy(&versions[VERSION_IDX_APP], &v, sizeof(version_t));
  }

  chThdCreateStatic(waThreadBlinker, sizeof(waThreadBlinker), NORMALPRIO, ThreadBlinker, NULL);
  chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO, ThreadBDU, NULL);
  chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO, ThreadSDU, NULL);

  while (TRUE)
  {
      while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);

      chThdSleepMilliseconds(100);

      if (usbConnected())
      {
        usbConnectBus(serusbcfg1.usbp);
      }
      else
      {
        usbDisconnectBus(serusbcfg1.usbp);
      }
    }
}
