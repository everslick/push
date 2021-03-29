#include <stdint.h>

#include <conio.h>
#include <spectrum.h>

#include "zxspec.h"

static uint8_t cursor_onoff = 0;

static uint8_t color_bg = 0;

void gotoxy(unsigned int x, unsigned int y) {
  zx_movecursorto(x, y);
}

uint8_t cursor(uint8_t onoff) {
  uint8_t old = cursor_onoff;

  cursor_onoff = onoff;

  return (old);
}

uint8_t bgcolor(uint8_t color) {
  uint8_t old = color_bg;

  color_bg = color;

  zx_setpaper(color);

  return (old);
}

void cclear(uint8_t length) {
  uint8_t i;

  for (i=0; i<length; i++) cputc(' ');
}

void waitvsync() {
}
