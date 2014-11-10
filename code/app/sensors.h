#ifndef SENSORS_H
#define SENSORS_H

#include "ch.h"
#include "hal.h"
#include "protocol.h"
#include "drivers.h"
#include "arm_math.h"
#include "median.h"
#include "ipc.h"
#include "common.h"

#define ADC_GRP1_NUM_CHANNELS   3
#define ADC_GRP1_BUF_DEPTH      128

#define ADC_GRP2_NUM_CHANNELS   1
#define ADC_GRP2_BUF_DEPTH      1024

#define AN_RATIO 1.61 /* 6600mV/4096 voltage divider ratio is 1.5 */
#define VBAT_RATIO 9.0f /* 36300mV/4096 voltage divider ratio is 9 */

//#define FFT_SIZE 512 // 4096-2048-1024-512-256-64-16 lengths supported by DSP library
//#define FFT_FREQ 117263

extern adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
extern adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
extern uint8_t output_knock[SPECTRUM_SIZE];

extern const ADCConversionGroup adcgrpcfg_sensors;
extern const ADCConversionGroup adcgrpcfg_knock;

extern sensors_t sensors_data;
extern TIMCAPConfig tc_conf;
extern monitor_t monitoring;

uint16_t calculateTpFromMillivolt(uint16_t AnMin, uint16_t AnMax, uint16_t AnVal);
uint16_t calculateRpmFromHertz(uint16_t freq);
uint16_t calculateKnockIntensity(uint16_t tgtFreq, uint16_t smplFreq, uint16_t qFactor, uint16_t* buffer, uint16_t size);

#endif
