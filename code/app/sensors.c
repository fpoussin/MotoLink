#include "sensors.h"
#include "limits.h"

adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
q15_t data_knock[sizeof(samples_knock)/2];
q15_t mag_knock[sizeof(samples_knock)/2];
uint8_t output_knock[FFT_SIZE];

sensors_t sensors_data = {0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F,0x0F0F};
uint8_t TIM3CC1CaptureNumber, TIM3CC2CaptureNumber;
uint16_t TIM3CC1ReadValue1, TIM3CC1ReadValue2;
uint16_t TIM3CC2ReadValue1, TIM3CC2ReadValue2;
bool knockDataReady = false;

void captureOverflowCb(TIMCAPDriver *timcapp)
{
  (void)timcapp;

  if ((TIM3->DIER & TIM_DIER_CC1IE)
      && TIM3CC1CaptureNumber == 0) {
    TIM3->DIER &= ~TIM_DIER_CC1IE;
    sensors_data.freq1 = 0;
  }

  if ((TIM3->DIER & TIM_DIER_CC2IE)
      && TIM3CC2CaptureNumber == 0) {
    TIM3->DIER &= ~TIM_DIER_CC2IE;
    sensors_data.freq2 = 0;
  }

}

void capture1Cb(TIMCAPDriver *timcapp)
{
  (void)timcapp;

  if(TIM3CC1CaptureNumber == 0)
  {
    /* Get the Input Capture value */
    TIM3CC1ReadValue1 = TIM3->CCR1;
    TIM3CC1CaptureNumber = 1;
  }
  else if(TIM3CC1CaptureNumber == 1)
  {
      uint32_t Capture;
      /* Get the Input Capture value */
      TIM3CC1ReadValue2 = TIM3->CCR1;

      /* Capture computation */
      if (TIM3CC1ReadValue2 > TIM3CC1ReadValue1)
      {
          Capture = ((uint32_t)TIM3CC1ReadValue2 - (uint32_t)TIM3CC1ReadValue1);
      }
      else
      {
          Capture = (((uint32_t)TIM3CC1ReadValue2 + 0x10000) - (uint32_t)TIM3CC1ReadValue1);
      }

      /* Frequency computation */
      sensors_data.freq1 = (tc_conf.frequency / Capture);

      TIM3CC1ReadValue1 = TIM3CC1ReadValue2;
      TIM3CC1CaptureNumber = 0;

      /* Disable CC1 interrupt */
      TIM3->DIER &= ~TIM_DIER_CC1IE;
  }
}

void capture2Cb(TIMCAPDriver *timcapp)
{
  (void)timcapp;
  if(TIM3CC2CaptureNumber == 0)
  {
    /* Get the Input Capture value */
    TIM3CC2ReadValue1 = TIM3->CCR2;
    TIM3CC2CaptureNumber = 1;
  }
  else if(TIM3CC2CaptureNumber == 1)
  {
      uint32_t Capture;
      /* Get the Input Capture value */
      TIM3CC2ReadValue2 = TIM3->CCR2;

      /* Capture computation */
      if (TIM3CC2ReadValue2 > TIM3CC2ReadValue1)
      {
          Capture = ((uint32_t)TIM3CC2ReadValue2 - (uint32_t)TIM3CC2ReadValue1);
      }
      else
      {
          Capture = (((uint32_t)TIM3CC2ReadValue2 + 0x10000) - (uint32_t)TIM3CC2ReadValue1);
      }

      /* Frequency computation */
      sensors_data.freq2 = (tc_conf.frequency / Capture);

      TIM3CC2ReadValue1 = TIM3CC2ReadValue2;
      TIM3CC2CaptureNumber = 0;

      /* Disable CC2 interrupt */
      TIM3->DIER &= ~TIM_DIER_CC2IE;
  }
}

TIMCAPConfig tc_conf = {
   {TIMCAP_INPUT_ACTIVE_HIGH, TIMCAP_INPUT_ACTIVE_HIGH, TIMCAP_INPUT_DISABLED, TIMCAP_INPUT_DISABLED},
   50000, /* TIM3 Runs at 36Mhz max. (1/50000)*65536 = 1.31s Max */
   {capture1Cb, capture2Cb, NULL, NULL},
   captureOverflowCb,
   0
};

/* Every 32 samples at 2.53Kz each, triggers at around 79Hz */
void sensorsCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  (void)adcp;
  uint16_t i, pos;
  uint32_t an[3] = {0, 0, 0};

  /* n is always depth/2 */
  for (i=0; i<n; i++)
  {
    pos = i * ADC_GRP1_NUM_CHANNELS;
    an[0] += buffer[pos];
    an[1] += buffer[pos+1];
    an[2] += buffer[pos+2];
  }

  /* n >> 4 */
  an[0] /= n;
  an[1] /= n;
  an[2] /= n;

  an[0] *= VBAT_RATIO;
  an[1] *= AN_RATIO;
  an[2] *= AN_RATIO;

  sensors_data.an7 = an[0] & 0xFFF0;
  sensors_data.an8 = an[1] & 0xFFF0;
  sensors_data.an9 = an[2] & 0xFFF0;
}

/* ADC12 Clk is 72Mhz/128 562Khz  */
const ADCConversionGroup adcgrpcfg_sensors = {
  TRUE,
  ADC_GRP1_NUM_CHANNELS,
  sensorsCallback,
  NULL,
  0,                        /* CFGR    */
  ADC_TR(0, 4095),          /* TR1     */
  ADC_CCR_TSEN | ADC_CCR_VBATEN, /* CCR     */
  {                         /* SMPR[2] */
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_61P5) | /* Sampling rate = 562000/(61.5+12.5) = 7.56Khz  */
    ADC_SMPR1_SMP_AN8(ADC_SMPR_SMP_61P5) | /* 7.56Khz for 3 channels = 2.53Khz */
    ADC_SMPR1_SMP_AN9(ADC_SMPR_SMP_61P5),
    0
  },
  {                         /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN7) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN8) |
    ADC_SQR1_SQ3_N(ADC_CHANNEL_IN9) | 0,
    0,
    0,
    0
  }
};

/* Every 512 samples at 112.5KHz each, triggers at around 220Hz */
void knockCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
  (void)adcp;
  uint32_t i;
  const int32_t* samples = (int32_t*)buffer;
  int32_t* data = (int32_t*)data_knock;

  /* n is always depth/2 */
  if (n % 16)
	  return;

  /* Copy to signed array 32bits at a time */
  /* ADC has offset setup and outputs Q15 values directly */
  /*
  for (i=0; i<n/2; i+=4)
  {
	data[i] = samples[i];
	data[i+1] = samples[i+1];
	data[i+2] = samples[i+2];
	data[i+3] = samples[i+3];

  }
*/
  for (i=0; i<n; i++)
  {
    data_knock[i] = buffer[i];
  }

  // Do FFT + Mag in a thread
  knockDataReady = true;
}

/* ADC34 Clk is 72Mhz/32 2.25Mhz  */
const ADCConversionGroup adcgrpcfg_knock = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  knockCallback,
  NULL,
  ADC_CFGR_ALIGN,                   /* CFGR    */
  ADC_TR(0, 4095),                  /* TR1     */
  0,    /* CCR     */
  {                                 /* SMPR[2] */
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_7P5),  /* Sampling rate = 2250000/(7.5+12.5) = 112.5Khz  */
    0,
  },
  {                                 /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
    0,
    0,
    0
  }
};
