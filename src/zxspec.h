#ifndef _ZXSPEC_H_
#define _ZXSPEC_H_

#include <stdint.h>

#define COLOR_BLACK        0
#define COLOR_BLUE         1
#define COLOR_RED          2
#define COLOR_PURPLE       3
#define COLOR_GREEN        4
#define COLOR_CYAN         5
#define COLOR_YELLOW       6
#define COLOR_WHITE        7

#define COLOR_DEFAULT      4

#define bgcolor  textbackground
#define cursor(onoff)

void waitvsync(void);

uint8_t revers(uint8_t onoff);

#endif // _ZXSPEC_H_
