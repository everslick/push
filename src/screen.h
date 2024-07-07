#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <stdint.h>

#ifdef OSCAR64

#include <vic.h>

#define COLOR_BLACK         VCOL_BLACK
#define COLOR_RED           VCOL_RED
#define COLOR_GREEN         VCOL_GREEN
#define COLOR_YELLOW        VCOL_YELLOW
#define COLOR_BLUE          VCOL_BLUE
#define COLOR_MAGENTA       VCOL_PURPLE
#define COLOR_CYAN          VCOL_CYAN
#define COLOR_WHITE         VCOL_WHITE

#define COLOR_ORANGE        VCOL_ORANGE
#define COLOR_BROWN         VCOL_BROWN
#define COLOR_RED2          VCOL_LT_RED
#define COLOR_GRAY          VCOL_DARK_GREY
#define COLOR_GRAY2         VCOL_MED_GREY
#define COLOR_GREEN2        VCOL_LT_GREEN
#define COLOR_BLUE2         VCOL_LT_BLUE
#define COLOR_GRAY3         VCOL_LT_GREY

#endif

#ifdef KICKC

#define COLOR_BLACK         0
#define COLOR_WHITE         1
#define COLOR_RED           2
#define COLOR_CYAN          3
#define COLOR_PURPLE        4
#define COLOR_GREEN         5
#define COLOR_BLUE          6
#define COLOR_YELLOW        7

#define COLOR_ORANGE        8
#define COLOR_BROWN         9
#define COLOR_PINK          10
#define COLOR_GRAY          11
#define COLOR_GRAY2         12
#define COLOR_GREEN2        13
#define COLOR_BLUE2         14
#define COLOR_GRAY3         15

#endif

void clrscr(void);
void screensize(uint8_t *x, uint8_t *y);

void gotoxy(uint8_t x, uint8_t y);

uint8_t wherex(void);
uint8_t wherey(void);

void cputc(char c);
void cputs(const char *s);
char cgetc(void);

uint8_t revers(uint8_t onoff);

uint8_t bgcolor(uint8_t color);
uint8_t bordercolor(uint8_t color);

#ifdef OSCAR64

#define cgetc   getchx
#define cputc   putch
#define cprintf printf

void cursor(uint8_t onoff);

void waitvsync(void);

int sleep(int seconds);

#else

uint8_t cursor(uint8_t onoff);
uint8_t textcolor(uint8_t color);

#endif

#endif // _SCREEN_H_
