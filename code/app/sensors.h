#ifndef SENSORS_H
#define SENSORS_H

#include "ch.h"
#include "hal.h"
#include "communication.h"
#include "timcap.h"

#define ADC_GRP1_NUM_CHANNELS   3
#define ADC_GRP1_BUF_DEPTH      1

#define ADC_GRP2_NUM_CHANNELS   1
#define ADC_GRP2_BUF_DEPTH      256

#define VOLT_RATIO 1.611328125f

extern adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
extern adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];

extern const ADCConversionGroup adcgrpcfg_sensors;
extern const ADCConversionGroup adcgrpcfg_knock;

extern sensors_t sensors_data;
extern TIMCAPConfig tc_conf;

void startCapture(void);

#endif
