#ifndef _LINUX_H_
#define _LINUX_H_

#define COLOR_BLACK           0
#define COLOR_RED             1
#define COLOR_GREEN           2
#define COLOR_YELLOW          3
#define COLOR_BLUE            4
#define COLOR_MAGENTA         5
#define COLOR_CYAN            6
#define COLOR_WHITE           7

#define COLOR_DEFAULT         9

// printf should go through cprintf
#define printf cprintf

#include <stdint.h>

#include "condrv.h"

uint8_t linux_init();
void    linux_fini();

#endif // _LINUX_H_
