#include "term.h"

#ifdef OSCAR64

static uint8_t revers_onoff = 0;

void waitvsync(void) {
  vic_waitFrame();
}

void cls(void) {
  uint8_t *SCREEN = (uint8_t *)0x0400;
  uint8_t *COLOR  = (uint8_t *)0xd800;

  memset(SCREEN, 0x20,          1000);
  memset(COLOR,  COLOR_DEFAULT, 1000);
}

uint8_t revers(uint8_t onoff) {
  uint8_t old = revers_onoff;

  revers_onoff = onoff;

  cputc((onoff) ? 18 : 148);

  return (old);
}

int sleep(int seconds) {
  for (int i=0; i<seconds*50; i++) {
    waitvsync();
  }

  return (0);
}

void screensize(uint8_t *x, uint8_t *y) {
  *x = 40; *y = 25;
}

void cursor(uint8_t onoff) {
  textcursor(onoff);
}

uint8_t bgcolor(uint8_t color) {
  uint8_t col = vic.color_back;

  vic.color_back = color;

  return (col);
}

uint8_t bordercolor(uint8_t color) {
  uint8_t col = vic.color_border;

  vic.color_border = color;

  return (col);
}

#endif
