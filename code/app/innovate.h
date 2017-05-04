/*
 * innovate.h
 *
 *  Created on: 2 nov. 2014
 *      Author: Mobyfab
 */

#ifndef INNOVATE_H_
#define INNOVATE_H_

#include "hal.h"

#define MTS_HEADER_MASK ((uint16_t)0xA280) /* Bits 13, 9, 7 */
#define MTS_LENGTH_LO ((uint16_t)0x007F) /* Bits 0-7 */
#define MTS_LENGTH_HI ((uint16_t)0x0100) /* Bit 9 */


#define MTS_STATUS_MASK_TEST_LC2 ((uint16_t)0xE200) /* Bits 15, 13, 9 */
#define MTS_STATUS_MASK_LC2 ((uint16_t)0x4200) /* Bit 15 */
#define MTS_STATUS_MASK ((uint16_t)0x1C00) /* Bits 2-4 */
#define MTS_STATUS_LAMBDA_WARMING ((uint16_t)0x1000) /* Bit 4 */
#define MTS_STATUS_LAMBDA_ERROR ((uint16_t)0x1800) /* Bits 3-4 */
#define MTS_STATUS_LAMBDA_NEED_CAL ((uint16_t)0x0C00) /* Bits 2-3 */

#define MTS_AFR_MUL_HI ((uint16_t)0x0100) /* Bit 0 */
#define MTS_AFR_MUL_LO ((uint16_t)0x007F) /* Bits 0-6 */

#define MTS_LAMBDA_HI ((uint16_t)0x3F00) /* Bits 0-5 */
#define MTS_LAMBDA_LO ((uint16_t)0x007F) /* Bits 0-6 */

void readMtsPackets(uint8_t *buf);

#endif /* INNOVATE_H_ */
