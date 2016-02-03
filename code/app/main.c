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

/*===========================================================================*/
/* Macros                                                                    */
/*===========================================================================*/

/* Check if tp was the previous thread */
#define RUNNING(tp) (uint16_t)((tp == pThreadMonitor->p_next) << 15)
#define FREQIN_INTERVAL MS2ST(50)

/*===========================================================================*/
/* Thread pointers.                                                          */
/*===========================================================================*/

thread_t *pThreadMonitor = NULL;
uint16_t irq_pct = 0;
const char *irq_name = "Interrupts";

/*===========================================================================*/
/* Structs / Vars                                                            */
/*===========================================================================*/

static virtual_timer_t vt_freqin;
static const uint8_t initMsg[] = {0xFE, 0x04, 0xFF, 0xFF}; // original HDS init msg

static const uint8_t initHex1[] = {0x27, 0x0B, 0xE0, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x48, 0x6F, 0x43}; // HelloHo
static const uint8_t initHex2[] = {0x07 , 0x08 , 0x49 , 0x7F , 0x61 , 0x6D , 0x7F, 0xDC}; // Iam
static const uint8_t initHex3[] = {0x27 , 0x0B , 0xE0 , 0x77 , 0x41 , 0x72 , 0x65 , 0x59 , 0x6F , 0x75 , 0x22}; // AreYou
static const uint8_t initHex4[] = {0x07 , 0x08 , 0x46 , 0x69 , 0x6E , 0x2E , 0x7F , 0x27}; // in.

bool recording = false;

/*===========================================================================*/
/* CallBacks                                                                 */
/*===========================================================================*/

CCM_FUNC void iwdgGptCb(GPTDriver *gptp)
{
  (void) gptp;
  wdgResetI(&WDGD1);
}

CCM_FUNC void freqinVTHandler(void *arg)
{
  (void)arg;

  reEnableInputCapture(&TIMCAPD3);

  chSysLockFromISR();
  chVTSetI(&vt_freqin, FREQIN_INTERVAL, freqinVTHandler, NULL);
  chSysUnlockFromISR();
}

/*===========================================================================*/
/* Configs                                                                   */
/*===========================================================================*/

const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

GPTConfig gpt1Cfg =
{
  100000,      /* timer clock.*/
  iwdgGptCb,  /* Timer callback.*/
  0,
  0
};

const DACConfig dac1cfg1 = {
  2047U,
  DAC_DHRM_12BIT_RIGHT
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

/*===========================================================================*/
/* Threads                                                                   */
/*===========================================================================*/

THD_WORKING_AREA(waThreadCAN, 256);
CCM_FUNC static THD_FUNCTION(ThreadCAN, arg)
{
  event_listener_t el;
  CANRxFrame rxmsg;

  (void)arg;
  chRegSetThreadName("CAN Bus");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);

  while(!chThdShouldTerminateX()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(50)) == 0) {
      chThdSleepMilliseconds(2);
      continue;
    }
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, MS2ST(50)) == MSG_OK) {
      /* Process message.*/
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
  return;
}

/*
 * USB Bulk thread.
 */
THD_WORKING_AREA(waThreadBDU, 700);
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

    pwmEnableChannel(&PWMD_LED2, CHN_LED2, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED2, idle_duty));

    if (flags & CHN_INPUT_AVAILABLE)
    {
      pwmEnableChannel(&PWMD_LED2, CHN_LED2, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED2, 8000));
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
THD_WORKING_AREA(waThreadSDU, 1024);
CCM_FUNC static THD_FUNCTION(ThreadSDU, arg)
{
  (void)arg;
  uint8_t buffer[SERIAL_BUFFERS_SIZE/2];
  //uint8_t buffer_check[SERIAL_BUFFERS_SIZE/2];
  size_t read;
  chRegSetThreadName("SDU");

  while(USBD1.state != USB_READY) chThdSleepMilliseconds(10);
  while(SDU1.state != SDU_READY) chThdSleepMilliseconds(10);
  while(SD1.state != SD_READY) chThdSleepMilliseconds(10);

  // Enable K-line
  palSetPad(PORT_KLINE_CS, PAD_KLINE_CS);

  while (TRUE) {

    while(SD1.state != SD_READY) chThdSleepMilliseconds(10);
/*
    if (doKLineInit && 0)
    {
      //klineInit();
      fiveBaudInit(&SD1);
      sdReadTimeout(&SD1, buffer_check, 1, MS2ST(5)); // noise
      doKLineInit = false;
	}
*/
    //pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 1000));
    read = chnReadTimeout(&SDU1, buffer, sizeof(buffer), MS2ST(5));
    if (read > 0)
    {
      //pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 8000));
      sdWriteTimeout(&SD1, buffer, read, MS2ST(100));
    }

    //pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 1000));
    read = sdReadTimeout(&SD1, buffer, sizeof(buffer), MS2ST(5));
    if (read > 0)
    {
      //pwmEnableChannel(&PWMD_LED1, CHN_LED1, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED1, 8000));
      chnWriteTimeout(&SDU1, buffer, read, MS2ST(100));
    }

    chThdSleepMilliseconds(1);
  }
  return;
}

/*
 * Sensors thread.
 */
static pair_t an1_buffer[ADC_GRP1_BUF_DEPTH/2];
static pair_t an2_buffer[ADC_GRP1_BUF_DEPTH/2];
static pair_t an3_buffer[ADC_GRP1_BUF_DEPTH/2];
THD_WORKING_AREA(waThreadADC, 128);
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

    sensors_data.an7 = an[0];
    sensors_data.an8 = an[1];
    sensors_data.an9 = an[2];

    /* Todo: get params from memory */
    sensors_data.tps = calculateTpFromMillivolt(500, 4500, sensors_data.an8);
    sensors_data.rpm = calculateRpmFromHertz(sensors_data.freq1, 100);
    sensors_data.afr = calculateAFRFromMillivolt(73, 224, sensors_data.an9);

    if (findCell(sensors_data.tps/2, sensors_data.rpm, &row, &col))
    {
      sensors_data.cell.row = row;
      sensors_data.cell.col = col;

      /* Average */
      tableAFR[row][col] = tableAFR[row][col] == 0 ? sensors_data.afr : ((uint16_t)sensors_data.afr+(uint16_t)tableAFR[row][col])/2;
      /* Peaks */
      tableKnock[row][col] = sensors_data.knock_value > tableKnock[row][col] ? sensors_data.knock_value : tableKnock[row][col];
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

    // TODO: Get freq and ratio from settings
    uint16_t freq = 8500;
    uint16_t ratio = 3000;
    sensors_data.knock_value = calculateKnockIntensity(freq, ratio, FFT_FREQ, output_knock, sizeof(output_knock));
    sensors_data.knock_freq = freq;
  }
  return;
}

/* Pointer to the idle thread */
static inline thread_t *chThdGetIdleX(void) {

  return ch.rlist.r_queue.p_prev;
}

/*
 * CPU Load Monitoring thread.
 */
THD_WORKING_AREA(waThreadMonitor, 256);
CCM_FUNC static THD_FUNCTION(ThreadMonitor, arg)
{
  (void)arg;
  chRegSetThreadName("Monitor");
  pThreadMonitor = chThdGetSelfX();
  uint32_t  run_offset, irq_ticks = 0, total_ticks;
  thread_t* tp = NULL;

  DWT->CTRL |= DWT_CTRL_EXCEVTENA_Msk;

  while (TRUE)
  {
    //chSysLock();

    tp = chRegFirstThread();
    do {
        tp->runtime = 0;
        tp->irqtime = 0;
        tp = chRegNextThread(tp);
    } while (tp != NULL);

    run_offset = DWT->CYCCNT;

    //chSysUnlock();

	/* Populate load data */
	chThdSleepMilliseconds(250);

	//chSysLock();

	/* Convert to systick time base */
	total_ticks = (DWT->CYCCNT - run_offset) / (STM32_SYSCLK/CH_CFG_ST_FREQUENCY);

    tp = chRegFirstThread();
    do {
        irq_ticks += tp->irqtime;
        tp = chRegNextThread(tp);
    } while (tp != NULL);

	//chSysUnlock();

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
  chRegSetThreadName("SER2");

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

THD_WORKING_AREA(waThreadButton, 64);
CCM_FUNC static THD_FUNCTION(ThreadButton, arg)
{
    (void)arg;
    uint8_t count = 0;
    while (true)
    {
        if (palReadPad(PORT_BUTTON1, PAD_BUTTON1) == PAL_LOW)
        {
            count++;
        }
        else
        {
            count = 0;
        }

        if (count > 10)
        {
            /* Toggle Record mode */
            recording = !recording;
        }

        chThdSleepMilliseconds(100);
    }
   return;
}


THD_WORKING_AREA(waThreadRecord, 128);
CCM_FUNC static THD_FUNCTION(ThreadRecord, arg)
{
    (void)arg;
    while (true)
    {
        if (recording)
        {
            palTogglePad(PORT_LED1, PAD_LED1);
            /* Record code */

        }
        chThdSleepMilliseconds(500);
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

  usbDisconnectBus(&USBD1);

  gptStart(&GPTD1, &gpt1Cfg);
  gptStartContinuous(&GPTD1, 20000);

  pwmStart(&PWMD_LED2, &pwmcfg);
  pwmEnableChannel(&PWMD_LED2, CHN_LED2, PWM_PERCENTAGE_TO_WIDTH(&PWMD_LED2, 8000));

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

  adcSTM32EnableTS(&ADCD1);
  adcSTM32EnableVBAT(&ADCD1);

  adcSTM32EnableTS(&ADCD3);
  adcSTM32EnableVBAT(&ADCD3);

  /* ADC 3 Ch1 Offset. -2048 */
  ADC3->OFR1 = ADC_OFR1_OFFSET1_EN | ((1 << 26) & ADC_OFR1_OFFSET1_CH) | (2048 & 0xFFF);
  dacPutChannelX(&DACD1, 0, 2048); // This sets the offset for the knock ADC opamp.
  adcStartConversion(&ADCD1, &adcgrpcfg_sensors, samples_sensors, ADC_GRP1_BUF_DEPTH);
  adcStartConversion(&ADCD3, &adcgrpcfg_knock, samples_knock, ADC_GRP2_BUF_DEPTH);
  timcapEnable(&TIMCAPD3);

  chVTSet(&vt_freqin, FREQIN_INTERVAL, freqinVTHandler, NULL);

  /*
   * Creates the threads.
   */
  chThdCreateStatic(waThreadBDU, sizeof(waThreadBDU), NORMALPRIO+1, ThreadBDU, NULL);
  chThdCreateStatic(waThreadSDU, sizeof(waThreadSDU), NORMALPRIO+2, ThreadSDU, NULL);
  chThdCreateStatic(waThreadADC, sizeof(waThreadADC), NORMALPRIO, ThreadADC, NULL);
  chThdCreateStatic(waThreadKnock, sizeof(waThreadKnock), NORMALPRIO, ThreadKnock, NULL);
  chThdCreateStatic(waThreadCAN, sizeof(waThreadCAN), NORMALPRIO, ThreadCAN, NULL);
  chThdCreateStatic(waThreadSER2, sizeof(waThreadSER2), NORMALPRIO, ThreadSER2, NULL);
  chThdCreateStatic(waThreadButton, sizeof(waThreadButton), NORMALPRIO+3, ThreadButton, NULL);
  chThdCreateStatic(waThreadRecord, sizeof(waThreadRecord), NORMALPRIO+3, ThreadRecord, NULL);
  /* Create last as it uses pointers from above */
  chThdCreateStatic(waThreadMonitor, sizeof(waThreadMonitor), NORMALPRIO+10, ThreadMonitor, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
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
