#ifndef _VT100_H_
#define _VT100_H_

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

uint8_t vt100_init();
void vt100_fini();

void clrscr(void);
void screensize(uint8_t *x, uint8_t *y);
void cclear(uint8_t length);

void gotoxy(uint8_t x, uint8_t y);

uint8_t wherex(void);
uint8_t wherey(void);

void cputc(char c);
void cputs(const char *s);
char cgetc(void);

int cprintf(const char *format, ...);
int vcprintf(const char *format, va_list ap);

uint8_t cursor(uint8_t onoff);
uint8_t textcolor(uint8_t color);
uint8_t bgcolor(uint8_t color);
uint8_t bordercolor(uint8_t color);

#endif // _VT100_H_
