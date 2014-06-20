#ifndef SENSORS_H
#define SENSORS_H

#include "ch.h"
#include "hal.h"
#include "protocol.h"
#include "drivers.h"
#include "arm_math.h"

#define ADC_GRP1_NUM_CHANNELS   3
#define ADC_GRP1_BUF_DEPTH      64

#define ADC_GRP2_NUM_CHANNELS   1
#define ADC_GRP2_BUF_DEPTH      1024 /* 2x512 for continuous FFT256 */

#define AN_RATIO 1.611328125f /* 6600mV/4096 */
#define VBAT_RATIO 8.056640625f /* 33000mV/4096 * TODO: Fix resistor values */

#define FFT_SIZE 256 // 4096-1024-256-64-16 lengths supported by DSP library
#define SAMPLING_RATE 112500

extern adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
extern adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
extern q15_t data_knock[sizeof(samples_knock)/2];
extern q15_t output_knock[sizeof(samples_knock)/2];

extern const ADCConversionGroup adcgrpcfg_sensors;
extern const ADCConversionGroup adcgrpcfg_knock;
extern bool knockDataReady;

extern sensors_t sensors_data;
extern TIMCAPConfig tc_conf;

#endif
