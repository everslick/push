#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#include "zxspec.h"

static uint8_t revers_onoff = 0;

void waitvsync(void) {
  int c = 500;

  while (c--);
}

uint8_t revers(uint8_t onoff) {
  uint8_t old = revers_onoff;

  revers_onoff = onoff;

  printf("\e[%um", (onoff) ? 7 : 27);

  return (old);
}
