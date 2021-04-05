#ifndef _CONDRV_H_
#define _CONDRV_H_

#include <stdarg.h>
#include <stdint.h>

void clrscr(void);
void screensize(uint8_t *x, uint8_t *y);

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

uint8_t kbhit(void);

#endif // _CONDRV_H_
