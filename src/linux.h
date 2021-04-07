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

#include <stdarg.h>
#include <stdint.h>

#include "screen.h"

#define printf cprintf

uint8_t linux_init(void);
void    linux_fini(void);

int cprintf(const char *format, ...);

#endif // _LINUX_H_
