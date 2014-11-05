/*
 * innovate.h
 *
 *  Created on: 2 nov. 2014
 *      Author: Mobyfab
 */

#ifndef INNOVATE_H_
#define INNOVATE_H_

#include "ch.h"
#include "hal.h"

#define MTS_HEADER_MASK ((uint8_t)0xA2) /* Bits 1, 5, 7 */
#define MTS_LENGTH_MASK1 ((uint8_t)0x01) /* Bit 0 */
#define MTS_LENGTH_MASK2 ((uint8_t)0x7F) /* Bits 0-6 */

#define MTS_STATUS_MASK ((uint8_t)0x1C) /* Bits 2-4 */
#define MTS_STATUS_LAMBDA_WARMING ((uint8_t)0x10) /* Bit 4 */
#define MTS_STATUS_LAMBDA_ERROR ((uint8_t)0x18) /* Bits 3-4 */
#define MTS_STATUS_LAMBDA_NEED_CAL ((uint8_t)0x0C) /* Bits 2-3 */

#define MTS_AFR_MUL_MASK1 ((uint8_t)0x01) /* Bit 0 */
#define MTS_AFR_MUL_MASK2 ((uint8_t)0x7F) /* Bits 0-6 */

#define MTS_LAMBDA_MASK1 ((uint8_t)0x3F) /* Bits 0-5 */
#define MTS_LAMBDA_MASK2 ((uint8_t)0x7F) /* Bits 0-6 */

void readMtsHeader(BaseChannel *chn, uint8_t *buf);

#endif /* INNOVATE_H_ */
