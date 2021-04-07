#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <stdint.h>

void clrscr(void);
void screensize(uint8_t *x, uint8_t *y);

void gotoxy(uint8_t x, uint8_t y);

uint8_t wherex(void);
uint8_t wherey(void);

void cputc(char c);
void cputs(const char *s);
char cgetc(void);

uint8_t cursor(uint8_t onoff);
uint8_t revers(uint8_t onoff);

uint8_t textcolor(uint8_t color);
uint8_t bgcolor(uint8_t color);
uint8_t bordercolor(uint8_t color);

#endif // _SCREEN_H_
