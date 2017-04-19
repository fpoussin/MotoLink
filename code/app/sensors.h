#ifndef SENSORS_H
#define SENSORS_H

#include "common.h"
#include "protocol.h"
#include "median.h"
#include "ipc.h"
#include "arm_math.h"

#define ADC_GRP1_NUM_CHANNELS   3
#define ADC_GRP1_BUF_DEPTH      128

#define ADC_GRP2_NUM_CHANNELS   1
#define ADC_GRP2_BUF_DEPTH      2048

#define VREF 3300
#define AN_RATIO 1.22f /* 5000mV/4096 voltage divider ratio is 1.22 (10K/2k=1.5) */
#define VBAT_RATIO 13.42f /* 55000mV/4096 voltage divider ratio is 13.42 (100K/10K=11)+(1K/2K=1.5) */

extern adcsample_t samples_sensors[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
extern adcsample_t samples_knock[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
extern uint8_t output_knock[SPECTRUM_SIZE];

extern const ADCConversionGroup adcgrpcfg_sensors;
extern const ADCConversionGroup adcgrpcfg_knock;

extern sensors_t sensors_data;
extern TIMCAPConfig tc_conf;
extern uint16_t irq_pct;
extern const char *irq_name;

void reEnableInputCapture(TIMCAPDriver *timcapp);

uint8_t calculateTpFromMillivolt(uint16_t AnMin, uint16_t AnMax, uint16_t AnVal);
uint8_t calculateAFRFromMillivolt(uint16_t afrMin, uint16_t afrMax, uint16_t AnVal);
uint16_t calculateFreqWithRatio(uint16_t freq, float32_t ratio);
uint8_t calculateKnockIntensity(uint16_t tgtFreq, uint16_t ratio, uint16_t smplFreq, uint8_t* buffer, uint16_t size);

#endif
