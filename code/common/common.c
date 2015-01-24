#include "common.h"

uint32_t leToInt(uint8_t *ptr) {

  return ((uint32_t)ptr[3] << 24) |
      ((uint32_t)ptr[2] << 16) |
      ((uint32_t)ptr[1] << 8) |
      (uint32_t)ptr[0];
}

uint32_t beToInt(uint8_t *ptr) {

  return ((uint32_t)ptr[0] << 24) |
      ((uint32_t)ptr[1] << 16) |
      ((uint32_t)ptr[2] << 8) |
      (uint32_t)ptr[3];
}

uint8_t checksum(const uint8_t *data, uint8_t length)
{
    uint8_t i;
    uint8_t sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}

bool getSwitch1(void)
{
	return palReadPad(SWITCH_PORT, SWITCH_PAD) == PAL_HIGH;
}

int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void klineInit(void)
{

  palSetPad(RELAY_DRV_PORT, RELAY_DRV_PAD);
  chThdSleepMilliseconds(70); // Low for 70ms
  palClearPad(RELAY_DRV_PORT, RELAY_DRV_PAD);
  chThdSleepMilliseconds(130); // High for 130ms

  /*
  // Set pin mode to GPIO
  palSetPadMode(KLINE_PORT, KLINE_RX, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(KLINE_PORT, KLINE_TX, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

  // Toggle K-line bus
  palClearPad(KLINE_PORT, KLINE_TX);
  chThdSleepMilliseconds(70); // Low for 70ms
  palSetPad(KLINE_PORT, KLINE_TX);
  chThdSleepMilliseconds(130); // High for 130ms

  // Set pin mode back to UART
  palSetPadMode(KLINE_PORT, KLINE_TX, PAL_MODE_ALTERNATE(7) | \
		  PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_PUDR_PULLUP);
  palSetPadMode(KLINE_PORT, KLINE_RX, PAL_MODE_ALTERNATE(7) | \
		  PAL_STM32_OTYPE_OPENDRAIN);
  */
}
