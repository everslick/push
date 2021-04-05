#include <stdint.h>

#include <conio.h>
#include <spectrum.h>

#include "zxnext.h"

static uint8_t cursor_onoff = 0;

static uint8_t color_bg = 0;

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
