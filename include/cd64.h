#ifndef CD64_H
#define CD64_H

#define CD64_REG_DATA_WR 0xb7800084
#define CD64_REG_DATA_RD 0xb7800080
#define CD64_REG_STATUS 0xb7800040
#define CD64_REG_CD_COMM_IO 0xb7800020
#define CD64_REG_CD_DATA_32 0xb7800018
#define CD64_REG_CD_DATA_16 0xb7800010
#define CD64_REG_MODE_ENABLE 0xb780000c
#define CD64_REG_MODE 0xb7800000

#define CD64_ENABLED 0xa
#define CD64_DISABLED 0x0

#define CD64_MODE_0 0
#define CD64_MODE_1 1
#define CD64_MODE_2 2
#define CD64_MODE_3 3
#define CD64_MODE_4 4
#define CD64_MODE_5 5
#define CD64_MODE_6 6
#define CD64_MODE_7 7

void cd64_init();
void cd64_enable();
void cd64_disable();

#endif // CD64_H
