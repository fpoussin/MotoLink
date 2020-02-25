#ifndef OBD_H
#define OBD_H

#include "ch.h"

#define OBD_MODE_QUERY_LIVEDATA 0x01
#define OBD_MODE_QUERY_FREEZEDATA 0x02
#define OBD_MODE_REPLY_LIVEDATA 0x41
#define OBD_MODE_REPLY_FREEZEDATA 0x42

#define OBD_PID_SUPPORT 0x00
#define OBD_PID_CODES 0x01
#define OBD_PID_FREEZE 0x02
#define OBD_PID_STATUS 0x03

#define OBD_PID_LOAD 0x04
#define OBD_PID_ECT 0x05
#define OBD_PID_MAP 0x0A
#define OBD_PID_RPM 0x0C
#define OBD_PID_SPEED 0x0D
#define OBD_PID_ADVANCE 0x0E
#define OBD_PID_IAT 0x0F
#define OBD_PID_MAF 0x10
#define OBD_PID_TPS 0x11
#define OBD_PID_AFR_CNT 0x13
#define OBD_PID_LAMBDA 0x14
#define OBD_PID_STANDARD 0x1C

#define OBD_PID_AFR 0x24
#define OBD_PID_SUPPORT2 0x20
#define OBD_PID_SUPPORT3 0x40

#define OBD_PID_VBAT 0x42
#define OBD_PID_ABS_LOAD 0x43

#define OBD_PID_SUPPORT4 0x60
#define OBD_PID_SUPPORT5 0x80
#define OBD_PID_SUPPORT6 0xA0
#define OBD_PID_SUPPORT7 0xC0

typedef struct {
  uint8_t len;
  uint8_t mode;
  uint8_t pid;
  uint8_t data[5];
} obd_msg_t;

#endif
