/*
 * innovate.c
 *
 *  Created on: 2 nov. 2014
 *      Author: Mobyfab
 */

#include "innovate.h"
#include "sensors.h"
#include "storage.h"
#include "usb_config.h"
#include "chprintf.h"

extern bool dbg_mts;

void readMtsPackets(uint8_t *buf)
{
  const uint16_t pkt_header = beToUInt16(&buf[0]);
  const uint16_t pkt_status_afr_mult = beToUInt16(&buf[2]);
  const uint16_t pkt_lambda = beToUInt16(&buf[4]);
  uint16_t len, status, afr_multiplier;
  uint16_t lambda, len_raw;
  uint8_t afr;

  // First, check header - packets are big endian so we had to convert them to little endian.
  if ((pkt_header & MTS_HEADER_MASK) == MTS_HEADER_MASK)
  {
    len_raw = ((pkt_header & MTS_LENGTH_HI) >> 1) | (pkt_header & MTS_LENGTH_LO);
    len = (len_raw * 2) + 2; // 2 bytes per packet + 2 bytes header

    // LC-1/2 messages are 2 words long, so 6 bytes total for 3 packets
    if (len == 6)
    {
      // Check message is from an LC-1/2
      if ((pkt_status_afr_mult & MTS_STATUS_MASK_TEST_LC1) != MTS_STATUS_MASK_LC1) {
        if (dbg_mts) chprintf(DBG_STREAM,
                              "->[MTS] Message not from an LC-1/2: %04X\r\n",
                              pkt_status_afr_mult);
        return;
      }

      status = pkt_status_afr_mult & MTS_STATUS_MASK;
      // Status is good
      if (status == 0)
      {
        /* Should be 147 */
        //afr_multiplier = ((pkt_status_afr_mult & MTS_AFR_MUL_HI) >> 1) | (pkt_status_afr_mult & MTS_AFR_MUL_LO);
        afr_multiplier = 147;

        /* Sensor value */
        lambda = ((pkt_lambda & MTS_LAMBDA_HI) >> 1) | (pkt_lambda & MTS_LAMBDA_LO);

        /* AFR Multiplied by 10 */
        afr = ((lambda + 500) * afr_multiplier) / 1000;
        if (dbg_mts) chprintf(DBG_STREAM,
                              "->[MTS] Multiplier/lambda/AFR(x10): %3u/%4u/%3u\r\n",
                              afr_multiplier, lambda, afr);
        if (settings.afrInput == AFR_INPUT_MTS && afr > 0)
            sensors_data.afr = afr;
      }
      else if (status & MTS_STATUS_LAMBDA_WARMING)
      {
        if (dbg_mts) chprintf(DBG_STREAM,
                              "->[MTS] Sensor warming up\r\n");
      }
      else if (status & MTS_STATUS_LAMBDA_NEED_CAL)
      {
        if (dbg_mts) chprintf(DBG_STREAM,
                              "->[MTS] Sensor need calibration\r\n");
      }
      else if (status & MTS_STATUS_LAMBDA_ERROR)
      {
        if (dbg_mts) chprintf(DBG_STREAM,
                              "->[MTS] Sensor error\r\n");
      }
      else
      {
        if (dbg_mts) chprintf(DBG_STREAM,
                              "->[MTS] Status error: %02x\r\n",
                              status);
      }
    }
    else {
      if (dbg_mts) chprintf(DBG_STREAM,
                            "->[MTS] Length incorrect: %02X, expecting 2\r\n",
                            len);
      return;
    }
  }
  else {
    if (dbg_mts) chprintf(DBG_STREAM,
                          "->[MTS] Not an MTS header: %04X/%04X\r\n",
                          pkt_header,
                          pkt_header & MTS_HEADER_MASK);
    return;
  }
}
