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
#include "innovate.h"
#include "storage.h"
#include "canbus.h"
#include "arm_math.h"
#include "tables.h"
#include "chprintf.h"
#include "shell.h"

/*===========================================================================*/
/* Macros                                                                    */
/*===========================================================================*/

#define SHELL_SD SDU1

/* Check if tp was the previous thread */
#define RUNNING(tp) (uint16_t)((tp == chThdGetSelfX()->p_next) << 15)
#define FREQIN_INTERVAL MS2ST(50)

/*===========================================================================*/
/* Thread pointers.                                                          */
/*===========================================================================*/


/*===========================================================================*/
/* Structs / Vars                                                            */
/*===========================================================================*/

uint16_t irq_pct = 0;
const char *irq_name = "Interrupts";
static virtual_timer_t vt_freqin;
extern bool dbg_can;
extern const ShellCommand sh_commands[];

/*===========================================================================*/
/* CallBacks                                                                 */
/*===========================================================================*/

/*
 * Set from bootloader, here for reference.
static const WDGConfig wdgcfg = {
  STM32_IWDG_PR_64,
  STM32_IWDG_RL(250), // 250ms
  STM32_IWDG_WIN_DISABLED
};
*/
CCM_FUNC void freqinVTHandler(void *arg)
{
  (void)arg;

  reEnableInputCapture(&TIMCAPD3);

  chSysLockFromISR();
  chVTSetI(&vt_freqin, FREQIN_INTERVAL, freqinVTHandler, NULL);
  chSysUnlockFromISR();
}

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SHELL_SD,
  sh_commands
};

/*===========================================================================*/
/* Configs                                                                   */
/*===========================================================================*/

const DACConfig dac1cfg1 = {
  2047U,
  DAC_DHRM_12BIT_RIGHT,
  0
};

SerialConfig uart1Cfg =
{
 19200, // bit rate
 0,
 USART_CR2_STOP1_BITS,
 0
};

SerialConfig uart2Cfg =
{
 19200, // bit rate
 0,
 USART_CR2_STOP1_BITS,
 0
};

PWMConfig pwmcfg = {
  10000,    /* 10kHz PWM clock frequency.   */
  50,      /* Initial PWM period 10mS.       */
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

const CANConfig cancfg = {
  CAN_MCR_ABOM   | CAN_MCR_AWUM   | CAN_MCR_TXFP,
  CAN_BTR_SJW(1) | CAN_BTR_TS2(4) |
  CAN_BTR_TS1(5) | CAN_BTR_BRP(5)
};

/*===========================================================================*/
/* Threads                                                                   */
/*===========================================================================*/

THD_WORKING_AREA(waThreadCAN, 256);
CCM_FUNC static THD_FUNCTION(ThreadCAN, arg)
{
  event_listener_t el;
  CANTxFrame txmsg;
  CANRxFrame rxmsg;
  uint16_t i;

  (void)arg;
  chRegSetThreadName("CAN Bus");

  while(CAND1.state != CAN_READY) chThdSleepMilliseconds(10);
  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);

  chEvtRegister(&CAND1.rxfull_event, &el, 0);

  while(TRUE) {

    // Are we using coms for sensor data? If not just sleep.
    if (settings.sensorsInput != SENSORS_INPUT_COM && (settings.functions & FUNC_OBD) == 0) {

      chThdSleepMilliseconds(100);
      continue;
    }

    //checkCanFilters(&CAND1, &cancfg);

    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(10)) == 0) {
      continue;
    }

    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) {
      /* Process message.*/

      if (dbg_can) {
          chprintf((BaseSequentialStream *)&SDU1, "->[%08x][%02x]", rxmsg.SID, rxmsg.RTR);
          for(i = 0; i < 8; i++) {
              chprintf((BaseSequentialStream *)&SDU1, ":%02x", rxmsg.data8[i]);
          }
          chprintf((BaseSequentialStream *)&SDU1, "\n");
      }

      if (settings.functions & FUNC_OBD) {

        serveCanOBDPidRequest(&CAND1, &txmsg, &rxmsg);

        if (dbg_can) {
            chprintf((BaseSequentialStream *)&SDU1, "<-[%08x][%02x]", txmsg.SID, txmsg.RTR);
            for(i = 0; i < 8; i++) {
                chprintf((BaseSequentialStream *)&SDU1, ":%02x", txmsg.data8[i]);
            }
            chprintf((BaseSequentialStream *)&SDU1, "\n");
        }
      }

      if (settings.sensorsInput == SENSORS_INPUT_OBD_CAN && 0) {

        readCanOBDPidResponse(&rxmsg);
      }
      else if (settings.sensorsInput == SENSORS_INPUT_YAMAHA_CAN && 0) {

        readCanYamahaPid(&rxmsg);
      }
    }

    if (settings.sensorsInput == SENSORS_INPUT_OBD_CAN && 0) {

      // Request PIDs
      sendCanOBDFrames(&CAND1, &txmsg);
      chThdSleepMilliseconds(100); // ~10Hz
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
  return;
}

/*
 * USB Bulk thread.
 */
THD_WORKING_AREA(waThreadBDU, 1024);
CCM_FUNC static THD_FUNCTION(ThreadBDU, arg)
{
  event_listener_t el1;
  eventmask_t flags;
  (void)arg;
  chRegSetThreadName("BDU");

  chEvtRegisterMask(chnGetEventSource(&SDU2), &el1, ALL_EVENTS);

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(SDU2.state != SDU_READY) chThdSleepMilliseconds(10);

  while (TRUE)
  {
    chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE);
    flags = chEvtGetAndClearFlags(&el1);

    pwmEnableChannel(&PWMD_LED2, CHN_LED2, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED2, 8000));

    if (flags & CHN_INPUT_AVAILABLE)
    {
      pwmEnableChannel(&PWMD_LED2, CHN_LED2, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED2, 1000));
      readCommand((BaseChannel *)&SDU2);
    }
    else
      chThdSleepMilliseconds(2);
  }
  return;
}

/*
 * USB Serial thread.
 */
THD_WORKING_AREA(waThreadSDU, 256);
THD_WORKING_AREA(waThreadShell, 512);
CCM_FUNC static THD_FUNCTION(ThreadSDU, arg)
{
  (void)arg;
  uint8_t in_buffer[SERIAL_BUFFERS_SIZE];
  uint8_t out_buffer[SERIAL_BUFFERS_SIZE];
  //uint8_t buffer_check[SERIAL_BUFFERS_SIZE/2];
  size_t in, out;
  thread_t* th_shell = NULL;
  chRegSetThreadName("SDU");

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);
  while(SD1.state != SD_READY) chThdSleepMilliseconds(10);

  // Enable K-line transceiver
  palSetPad(PORT_KLINE_CS, PAD_KLINE_CS);

  shellInit();

  while (TRUE) {

    if (settings.serialMode == SERIAL_MODE_SHELL)  {
        if (th_shell == NULL || chThdTerminatedX(th_shell)) {
            th_shell = shellCreateStatic(&shell_cfg1, waThreadShell, sizeof(waThreadShell), NORMALPRIO +1);
        }
        chThdSleepMilliseconds(10);
        continue;
    }

    if (settings.serialMode != SERIAL_MODE_KLINE) {
        chThdSleepMilliseconds(10);
        continue;
    }

    /* In case we stop it to change baudrate */
    while (SD1.state != SD_READY) chThdSleepMilliseconds(10);

    if (doKLineInit && 0)
    {
      sdStop(&SD1);
      klineInit(false);
      //fiveBaudInit(&SD1);
      //sdReadTimeout(&SD1, buffer_check, 1, MS2ST(5)); // noise
      doKLineInit = false;
      sdStart(&SD1, &uart1Cfg);
    }

    in = chnReadTimeout(&SDU1, in_buffer, sizeof(in_buffer), TIME_IMMEDIATE);
    out = sdReadTimeout(&SD1, out_buffer, sizeof(out_buffer), TIME_IMMEDIATE);

    while (in == 0 && out == 0) {

        chThdSleepMilliseconds(1);
        in = chnReadTimeout(&SDU1, in_buffer, sizeof(in_buffer), TIME_IMMEDIATE);
        out = sdReadTimeout(&SD1, out_buffer, sizeof(out_buffer), TIME_IMMEDIATE);
    }

    if (in > 0)
    {
      sdWriteTimeout(&SD1, in_buffer, in, MS2ST(10));
    }

    if (out > 0)
    {
      chnWriteTimeout(&SDU1, out_buffer, out, MS2ST(10));
    }

  }
  return;
}

/*
 * Sensors thread.
 */
static pair_t an1_buffer[ADC_GRP1_BUF_DEPTH/2];
static pair_t an2_buffer[ADC_GRP1_BUF_DEPTH/2];
static pair_t an3_buffer[ADC_GRP1_BUF_DEPTH/2];
THD_WORKING_AREA(waThreadADC, 192);
CCM_FUNC static THD_FUNCTION(ThreadADC, arg)
{
  (void)arg;
  chRegSetThreadName("Sensors");

  adcsample_t * sensorsDataPtr;
  size_t n;
  uint16_t i, pos;
  uint32_t an[3] = {0, 0, 0};
  median_t an1, an2, an3;
  uint8_t row, col;

  chThdSleepMilliseconds(250);
  timcapEnable(&TIMCAPD3);
  chVTSet(&vt_freqin, FREQIN_INTERVAL, freqinVTHandler, NULL);
  adcStartConversion(&ADCD1, &adcgrpcfg_sensors, samples_sensors, ADC_GRP1_BUF_DEPTH);

  median_init(&an1, 0 , an1_buffer, ADC_GRP1_BUF_DEPTH/2);
  median_init(&an2, 0 , an2_buffer, ADC_GRP1_BUF_DEPTH/2);
  median_init(&an3, 0 , an3_buffer, ADC_GRP1_BUF_DEPTH/2);

  while (TRUE)
  {
    while (!recvFreeSamples(&sensorsMb, (void*)&sensorsDataPtr, &n))
      chThdSleepMilliseconds(5);

    an[0]= 0;
    an[1]= 0;
    an[2]= 0;

    /* Filtering and adding */
    for (i = 0; i < (n/ADC_GRP1_NUM_CHANNELS); i++)
    {
      pos = i * ADC_GRP1_NUM_CHANNELS;
      an[0] += median_filter(&an1, sensorsDataPtr[pos]);
      an[1] += median_filter(&an2, sensorsDataPtr[pos+1]);
      an[2] += median_filter(&an3, sensorsDataPtr[pos+2]);
    }

    /* Averaging */
    an[0] /= (n/ADC_GRP1_NUM_CHANNELS);
    an[1] /= (n/ADC_GRP1_NUM_CHANNELS);
    an[2] /= (n/ADC_GRP1_NUM_CHANNELS);

    /* Convert to milliVolts */
    an[0] *= VBAT_RATIO;
    an[1] *= AN_RATIO;
    an[2] *= AN_RATIO;

    sensors_data.an1 = an[0];
    sensors_data.an2 = an[1];
    sensors_data.an3 = an[2];

    /* Analog/Digital Sensors */
    if (settings.sensorsInput == SENSORS_INPUT_DIRECT) {
        sensors_data.tps = calculateTpFromMillivolt(settings.tpsMinV, settings.tpsMaxV, sensors_data.an2);
        sensors_data.rpm = calculateFreqWithRatio(sensors_data.freq1, settings.rpmMult);
        sensors_data.spd = calculateFreqWithRatio(sensors_data.freq2, settings.spdMult);
    }
    else if (settings.sensorsInput == SENSORS_INPUT_TEST) {
        sensors_data.tps = rand16(0, 200);
        sensors_data.rpm = rand16(10, 18000);
        sensors_data.spd = rand16(5, 10000);
    }

    /* AFR */
    if (settings.afrInput == AFR_INPUT_AN) {
        sensors_data.afr = calculateAFRFromMillivolt(settings.AfrMinVal, settings.AfrMaxVal, sensors_data.an3);
    }
    else if (settings.afrInput == AFR_INPUT_TEST) {
        sensors_data.afr = rand16(11000, 16000) / 100;
    }

    if (findCell(sensors_data.tps/2, sensors_data.rpm, &row, &col))
    {
      sensors_data.cell.row = row;
      sensors_data.cell.col = col;

      if (settings.functions & FUNC_RECORD)
      {
          /* Average */
          tableAFR[row][col] = tableAFR[row][col] == 0 ? sensors_data.afr : ((uint16_t)sensors_data.afr+(uint16_t)tableAFR[row][col])/2;
          /* Peaks */
          tableKnock[row][col] = sensors_data.knock_value > tableKnock[row][col] ? sensors_data.knock_value : tableKnock[row][col];
      }
    }
  }
  return;
}

/*
 * Knock processing thread.
 */
static float32_t input[FFT_SIZE*2];
static float32_t output[FFT_SIZE*2];
static float32_t mag_knock[FFT_SIZE/2];
THD_WORKING_AREA(waThreadKnock, 600);
CCM_FUNC static THD_FUNCTION(ThreadKnock, arg)
{
  (void)arg;
  chRegSetThreadName("Knock");

  q15_t* knockDataPtr;
  size_t knockDataSize;

  float32_t maxValue = 0;
  uint32_t maxIndex = 0;
  uint16_t i;

  /* ADC 3 Ch1 Offset. -2048 */
  ADC3->OFR1 = ADC_OFR1_OFFSET1_EN | ((1 << 26) & ADC_OFR1_OFFSET1_CH) | (2048 & 0xFFF);
  dacPutChannelX(&DACD1, 0, 2048); // This sets the offset for the knock ADC opamp.

  chThdSleepMilliseconds(200);
  adcStartConversion(&ADCD3, &adcgrpcfg_knock, samples_knock, ADC_GRP2_BUF_DEPTH);

  /* Initialize the CFFT/CIFFT module */
  arm_rfft_fast_instance_f32 S1;
  arm_rfft_fast_init_f32(&S1, FFT_SIZE);

  while (TRUE)
  {
    while (!recvFreeSamples(&knockMb, (void*)&knockDataPtr, &knockDataSize))
      chThdSleepMilliseconds(2);

    /* Copy and convert ADC samples */
    for (i=0; i<FFT_SIZE*2; i+=4)
    {
      /* Hann Window */
      float32_t multiplier = (1.0 - arm_cos_f32((2.0*PI*(float32_t)i)/(((float32_t)FFT_SIZE*2.0)-1.0)));
      input[i] = multiplier*(float32_t)knockDataPtr[i];
      input[i+1] = multiplier*(float32_t)knockDataPtr[i+1];
      input[i+2] = multiplier*(float32_t)knockDataPtr[i+2];
      input[i+3] = multiplier*(float32_t)knockDataPtr[i+3];
    }

    /* Process the data through the RFFT module */
    arm_rfft_fast_f32(&S1, input, output, 0);

    /* Process the data through the Complex Magnitude Module for
    calculating the magnitude at each bin */
    arm_cmplx_mag_f32(output, mag_knock, FFT_SIZE/2); // Calculate magnitude, outputs q2.14
    arm_max_f32(mag_knock, FFT_SIZE/2, &maxValue, &maxIndex); // Find max magnitude

    // Convert 2.14 to 8 Bits unsigned
    for (i=0; i < sizeof(output_knock); i++)
    {
      uint16_t tmp = (mag_knock[i]/16384);
      if (tmp > 0xFF)
        tmp = 0xFF;
      output_knock[i] = tmp; // 8 bits minus the 2 fractional bits
    }

    sensors_data.knock_freq = settings.knockFreq;

    if (settings.sensorsInput == SENSORS_INPUT_TEST) {
        sensors_data.knock_value = rand16(0, 255);
        continue;
      }

    sensors_data.knock_value = calculateKnockIntensity(
                settings.knockFreq,
                settings.knockRatio,
                FFT_FREQ,
                output_knock,
                sizeof(output_knock));
  }
  return;
}

/*
 * CPU Load Monitoring thread.
 */
THD_WORKING_AREA(waThreadMonitor, 128);
static THD_FUNCTION(ThreadMonitor, arg)
{
  (void)arg;
  chRegSetThreadName("Monitor");
  uint32_t  run_offset, irq_ticks = 0, total_ticks;
  thread_t* tp = NULL;

  DWT->CTRL |= DWT_CTRL_EXCEVTENA_Msk;

  while (TRUE)
  {
    tp = chRegFirstThread();
    do {
        tp->runtime = 0;
        tp->irqtime = 0;
        tp = chRegNextThread(tp);
    } while (tp != NULL);

    irq_ticks = 0;
    run_offset = DWT->CYCCNT;

	/* Populate load data */
    chThdSleepMilliseconds(1000);

	/* Convert to systick time base */
	total_ticks = (DWT->CYCCNT - run_offset) / (STM32_SYSCLK/CH_CFG_ST_FREQUENCY);

    tp = chRegFirstThread();
    do {
        irq_ticks += tp->irqtime;
        tp = chRegNextThread(tp);
    } while (tp != NULL);

    tp = chRegFirstThread();
    do {
        tp->pct = ((tp->runtime*10000)/total_ticks) | RUNNING(tp);
        tp = chRegNextThread(tp);
    } while (tp != NULL);

    irq_pct = ((irq_ticks*10000)/total_ticks);
  }
  return;
}

/*
 * Uart2 thread.
 */
THD_WORKING_AREA(waThreadSER2, 128);
CCM_FUNC static THD_FUNCTION(ThreadSER2, arg)
{
  (void)arg;
  uint8_t buffer[SERIAL_BUFFERS_SIZE/2];
  size_t read;
  chRegSetThreadName("MTS");

  while(SD2.state != SD_READY) chThdSleepMilliseconds(10);

  while (TRUE) {

    read = sdReadTimeout(&SD2, buffer, 1, TIME_IMMEDIATE);
    if (read > 0)
    {
      readMtsHeader((BaseChannel *)&SD2, buffer);
    }

    chThdSleepMilliseconds(1);
  }
  return;
}

THD_WORKING_AREA(waThreadRecord, 256);
static THD_FUNCTION(ThreadRecord, arg)
{
    (void)arg;
    chRegSetThreadName("Recording");
    uint16_t duty = 0;
    uint8_t result = 0;
    uint8_t count = 0;
    bool debounce = false;
    bool indicator = false;

    /* Load settings from EE first */
    if (readSettingsFromEE() != 0)
    {
        pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 10000));
        writeSettingsToEE();
        chThdSleepMilliseconds(3000);
    }

    if (readTablesFromEE() != 0)
    {
        pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 0));
        chThdSleepMilliseconds(1000);
        pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 10000));
        chThdSleepMilliseconds(3000);
    }

    while (true)
    {
        if (palReadPad(PORT_BUTTON1, PAD_BUTTON1) == PAL_LOW)
        {
            if (!debounce)
            {
                pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 10000));
                count++;
                indicator = true;
            }
            else {
                count = 0;
                indicator = false;
            }
        }
        else
        {
            count = 0;
            debounce = false;
            indicator = false;
        }

        if (count >= 2 && !debounce)
        {
            /* Toggle Record mode */
            settings.functions ^= FUNC_RECORD;
            writeSettingsToEE();
            readSettingsFromEE();
            debounce = true;

            pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 0));
            chThdSleepMilliseconds(150);
            pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 10000));
            chThdSleepMilliseconds(150);
            pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 0));
            chThdSleepMilliseconds(150);
            pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 10000));
            chThdSleepMilliseconds(150);
            pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 0));
        }

        if (settings.functions & FUNC_RECORD)
        {
            /* Record tables */
            result = writeTablesToEE();
            if (result == 0)
            {
                duty = duty == 0 ? 10000 : 0;
            }
            else
            {
                duty = duty == 0 ? 1000 : 0;
            }
        }
        else
        {
            duty = 0;
        }

        if (!indicator)
            pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, duty));

        chThdSleepMilliseconds(500);
    }
   return;
}

THD_WORKING_AREA(waThreadWdg, 64);
static THD_FUNCTION(ThreadWdg, arg)
{
    (void)arg;
    chRegSetThreadName("Watchdog");

    while (true)
    {
        wdgResetI(&WDGD1);
        chThdSleepMilliseconds(200);
    }

    return;
}

/*===========================================================================*/
/* Main Thread                                                               */
/*===========================================================================*/

int main(void)
{
  /*
   * Start OS and HAL
   */

  halInit();
  chSysInit();
  setupIPC();

  DEBUGEN(printf("App Mode\n"));

  usbDisconnectBus(&USBD1);

  /*
   * Initialize extra driver objects.
   */
  sduObjectInit(&SDU1);
  sduObjectInit(&SDU2);

  /*
   * Start peripherals
   */
  sdStart(&SD1, &uart1Cfg);
  sdStart(&SD2, &uart2Cfg);
  usbStart(&USBD1, &usbcfg);
  sduStart(&SDU1, &serusbcfg1);
  sduStart(&SDU2, &serusbcfg2);
  canStart(&CAND1, &cancfg);
  dacStart(&DACD1, &dac1cfg1);
  adcStart(&ADCD1, NULL);
  adcStart(&ADCD3, NULL);
  timcapStart(&TIMCAPD3, &tc_conf);
  pwmStart(&PWMD_LED2, &pwmcfg);

  eeInit();
  // Compare and update versions in EEprom if needed.
  version_t v;
  if (readVersionFromEE(VERSION_IDX_APP, &v) == 0 && memcmp(&versions, &v, sizeof(version_t)) != 0) {

    writeVersionToEE(VERSION_IDX_APP, &versions[VERSION_IDX_APP]);
  }
  if (readVersionFromEE(VERSION_IDX_BL, &v) == 0 ) {
    memcpy(&versions[VERSION_IDX_BL], &v, sizeof(version_t));
  }

  /*
   * Creates the threads.
   */
  chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO+5, ThreadBDU, NULL);
  chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO+2, ThreadSDU, NULL);
  chThdCreateStatic(waThreadADC, sizeof(waThreadADC), NORMALPRIO, ThreadADC, NULL);
  chThdCreateStatic(waThreadKnock, sizeof(waThreadKnock), NORMALPRIO, ThreadKnock, NULL);
  chThdCreateStatic(waThreadCAN, sizeof(waThreadCAN), NORMALPRIO, ThreadCAN, NULL);
  chThdCreateStatic(waThreadSER2, sizeof(waThreadSER2), NORMALPRIO, ThreadSER2, NULL);
  chThdCreateStatic(waThreadRecord, sizeof(waThreadRecord), NORMALPRIO+1, ThreadRecord, NULL);
  chThdCreateStatic(waThreadWdg, sizeof(waThreadWdg), HIGHPRIO, ThreadWdg, NULL);

  /* Create last as it uses pointers from above */
  chThdCreateStatic(waThreadMonitor, sizeof(waThreadMonitor), NORMALPRIO+10, ThreadMonitor, NULL);

  while (TRUE)
  {
    while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);

    chThdSleepMilliseconds(100);

    if (usbConnected())
    {
      usbConnectBus(&USBD1);
    }
    else
    {
      usbDisconnectBus(&USBD1);
    }
  }
}
