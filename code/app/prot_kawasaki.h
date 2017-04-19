#ifndef KAWASAKI_H
#define KAWASAKI_H

#define KAWASAKI_ECU_ADDR 0x11
#define KAWASAKI_MTL_ADDR 0xF2

#define KAWASAKI_CMD_COMREQ     0x81
#define KAWASAKI_CMD_COMSTRT    {0xC1, 0xEA, 0x8F}
#define KAWASAKI_CMD_DIAGREQ    {0x10, 0x80}
#define KAWASAKI_CMD_DIAGSTRT   {0x50, 0x80}
#define KAWASAKI_CMD_GETPID     0x21

#define KAWASAKI_PID_TPS 0x04 // 0% = 0x00 0xD8, 100% = 0x03 0x7F
#define KAWASAKI_PID_ECT 0x06 // (a - 48) / 1.6
#define KAWASAKI_PID_IAT 0x07 //  ^
#define KAWASAKI_PID_RPM 0x09 // (a * 100) + b
#define KAWASAKI_PID_GEAR 0x0B // a
#define KAWASAKI_PID_SPD 0x0C // (a << 8 + b) / 2

#endif
