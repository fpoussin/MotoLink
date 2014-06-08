#include "sensors.h"

adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];

sensors_t sensors_data = {0,0,0};

static void sensorsCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

  (void)adcp;
  (void)n;

  sensors_data.an7 = ((buffer[0]+buffer[3]+buffer[7])/3)*VOLT_RATIO;
  sensors_data.an8 = ((buffer[1]+buffer[4]+buffer[8])/3)*VOLT_RATIO;
  sensors_data.an9 = ((buffer[2]+buffer[5]+buffer[9])/3)*VOLT_RATIO;
}

/* ADC12 Clk is 72Mhz/128 562Khz  */
const ADCConversionGroup adcgrpcfg_sensors = {
  TRUE,
  ADC_GRP1_NUM_CHANNELS,
  sensorsCallback,
  NULL,
  0,                        /* CFGR    */
  ADC_TR(0, 4095),          /* TR1     */
  0, /* CCR     */
  {                         /* SMPR[2] */
    ADC_SMPR1_SMP_AN7(ADC_SMPR_SMP_19P5) | /* Sampling rate = 562000/(19.5+12.5) = 17.5Khz  */
    ADC_SMPR1_SMP_AN8(ADC_SMPR_SMP_19P5) |
    ADC_SMPR1_SMP_AN9(ADC_SMPR_SMP_19P5),
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

/* ADC34 Clk is 72Mhz/32 2.25Mhz  */
const ADCConversionGroup adcgrpcfg_knock = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  NULL,
  NULL,
  0,                                /* CFGR    */
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
