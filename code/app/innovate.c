/*
 * innovate.c
 *
 *  Created on: 2 nov. 2014
 *      Author: Mobyfab
 */

#include "innovate.h"
#include "sensors.h"

void readMtsHeader(BaseChannel *chn, uint8_t *buf)
{
  size_t read;
  uint8_t header = buf[0];
  uint8_t len, status, afr_multiplier;
  uint16_t lambda;
  // First, check header
  if ((header & MTS_HEADER_MASK) == MTS_HEADER_MASK)
  {

    // Get Packet length
    read = chnReadTimeout(chn, buf, 1, MS2ST(50));
    if (read < 1)
      return;

    len = ((header & MTS_LENGTH_MASK1) << 7)
        & (buf[0] & MTS_LENGTH_MASK2);

    // LC-1/2 messages are 2 words long
    if (len == 2)
    {
      read = chnReadTimeout(chn, buf, len*2, MS2ST(50));
      if (read != len*2)
        return;

      // Check message is from an LC-1/2
      if (!((buf[0] & 0xE2) == 0x42))
        return;

      status = buf[0] & MTS_STATUS_MASK;

      // Status is good
      if (status == 0)
      {
        /* Should be 147 */
        afr_multiplier = ((buf[0] & MTS_AFR_MUL_MASK1) << 7)
            & (buf[1] & MTS_AFR_MUL_MASK2);

        lambda = ((buf[2] & MTS_LAMBDA_MASK1) << 7)
            & (buf[3] & MTS_LAMBDA_MASK2);

        /* Multiplied by 10 */
        sensors_data.afr = ((lambda+500) * afr_multiplier) / 1000;
      }
    }
    else
      return;

  }
  else
    return;

}
