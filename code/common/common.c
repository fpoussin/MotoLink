
#include "common.h"
#include "string.h"
#include <stdlib.h>

inline uint32_t getuuid32(void) {

    return MCU_UUID[0] * MCU_UUID[1] * MCU_UUID[2];
}

uint16_t rand16(uint16_t min, uint16_t max)
{
    uint16_t r;
    const uint16_t range = 1 + max - min;
    const uint16_t buckets = (RAND_MAX & 0xFFFF) / range;
    const uint16_t limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = (rand() & 0xFFFF);
    } while (r >= limit);

    return min + (r / buckets);
}

uint32_t rand32(uint32_t min, uint32_t max)
{
    uint32_t r;
    const uint32_t range = 1 + max - min;
    const uint32_t buckets = RAND_MAX / range;
    const uint32_t limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

inline uint32_t leToUInt32(uint8_t *ptr) {

  return ((uint32_t)ptr[3] << 24) |
      ((uint32_t)ptr[2] << 16) |
      ((uint32_t)ptr[1] << 8) |
      (uint32_t)ptr[0];
}

inline uint16_t leToUInt16(uint8_t *ptr) {

  return ((uint16_t)ptr[1] << 8) | (uint16_t)ptr[0];
}

inline uint32_t beToUInt32(uint8_t *ptr) {

  return ((uint32_t)ptr[0] << 24) |
      ((uint32_t)ptr[1] << 16) |
      ((uint32_t)ptr[2] << 8) |
      (uint32_t)ptr[3];
}

inline uint16_t beToUInt16(uint8_t *ptr) {

  return ((uint16_t)ptr[0] << 8) | (uint16_t)ptr[1];
}

uint8_t checksum(const uint8_t *data, uint8_t length)
{
    uint8_t i;
    uint8_t sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}

inline bool getSwitch1(void)
{
    return palReadPad(PORT_BUTTON1, PAD_BUTTON1) == PAL_LOW;
}

inline int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#define K_LOW(time) palClearPad(PORT_KLINE_TX, PAD_KLINE_TX); chThdSleepMilliseconds(time);
#define K_HIGH(time) palSetPad(PORT_KLINE_TX, PAD_KLINE_TX); chThdSleepMilliseconds(time);

void klineInit(bool honda)
{
  // Set pin mode to GPIO
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_RX, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_TX, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

  // Toggle K-line bus
  if (honda) {
    // Honda HDS specific init
    K_LOW(70); // Low for 70ms
    K_HIGH(130); // High for 130ms
  }
  else {
    // KWP2000 Fast init (Kawasaki KDS)
    K_LOW(25); // Low for 25ms
    K_HIGH(25); // High for 25ms
  }

  // Set pin mode back to UART
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_TX, PAL_MODE_ALTERNATE(7) | \
          PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_RX, PAL_MODE_ALTERNATE(7) | \
                  PAL_STM32_OTYPE_OPENDRAIN);

}

bool fiveBaudInit(SerialDriver *sd)
{
  uint8_t input_buf[3];
  size_t bytes;
  // Set pin mode to GPIO
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_RX, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_TX, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

  const uint8_t prio = chThdGetPriorityX();
  chThdSetPriority(HIGHPRIO);

  K_HIGH(320); // As per ISO standard

  // Send 0x33 at 5 baud (00110011)
  // K-line level: |_--__--__-|
  K_LOW(200); // Low for 200ms - start bit
  K_HIGH(400); // High for 400ms - 00
  K_LOW(400); // Low for 400ms - 11
  K_HIGH(400); // High for 400ms - 00
  K_LOW(400); // Low for 400ms - 11
  K_HIGH(200); // High for 200ms - stop bit

  // Set pin mode back to UART
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_TX, PAL_MODE_ALTERNATE(7) | \
          PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_PUPDR_PULLUP);
  palSetPadMode(PORT_KLINE_TX, PAD_KLINE_RX, PAL_MODE_ALTERNATE(7) | \
          PAL_STM32_OTYPE_OPENDRAIN);

  chThdSetPriority(prio); // Revert back original priority

  chThdSleepMilliseconds(25);
  bytes = sdReadTimeout(sd, input_buf, sizeof(input_buf), TIME_MS2I(500)); // 300ms max according to ISO9141

  if (bytes != 3 || input_buf[0] != 0x55)
  {
      return false;
  }

  chThdSleepMilliseconds(35); // 25-50 ms pause per ISO standard
  uint8_t key = input_buf[2] ^ 0xFF; // Invert key byte
  sdWriteTimeout(sd, &key, 1, TIME_MS2I(100));

  chThdSleepMilliseconds(35); // 25-50 ms pause per ISO standard
  bytes = sdReadTimeout(sd, input_buf, 1, TIME_MS2I(100));
  if (bytes != 1 || input_buf[0] != 0xCC)
  {
      return false;
  }

  return true;
}

void setLineCoding(cdc_linecoding_t* lcp, SerialDriver *sdp, SerialConfig* scp)
{
  const uint32_t baudrate = (uint32_t)lcp->dwDTERate;

  scp->speed = baudrate;
  scp->cr1 = 0;
  scp->cr2 = 0;
  scp->cr3 = 0;

  switch (lcp->bCharFormat)
  {
    case 0:
      scp->cr2 = USART_CR2_STOP1_BITS;
      break;
    case 1:
      scp->cr2 = USART_CR2_STOP1P5_BITS;
      break;
    case 2:
      scp->cr2 = USART_CR2_STOP2_BITS;
      break;
    default:
      break;
  }

  // TODO
  switch (lcp->bParityType)
  {
    default:
      break;
  }

  // TODO
  switch (lcp->bDataBits)
  {
    default:
      break;
  }

  if (sdp->state != SD_UNINIT)
    return;

  while(sdp->state != SD_READY) chThdSleepMilliseconds(1);
  sdStop(sdp);

  while(sdp->state != SD_STOP) chThdSleepMilliseconds(1);
  sdStart(sdp, scp);
}

inline bool vbatDetect(void)
{
    return palReadPad(PORT_VCC_DETECT, PAD_VCC_DETECT) == PAL_LOW;
}
