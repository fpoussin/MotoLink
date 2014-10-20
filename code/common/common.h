#ifndef COMMON_H
#define COMMON_H

#include "ch.h"
#include "hal.h"

inline uint32_t leToInt(uint8_t *ptr);
inline uint32_t beToInt(uint8_t *ptr);
uint8_t checksum(uint8_t *data, uint8_t length);
bool getSwitch1(void);

#endif