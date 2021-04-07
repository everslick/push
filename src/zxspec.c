#include <stdint.h>

#include "zxspec.h"

static uint8_t revers_onoff = 0;

uint8_t revers(uint8_t onoff) {
  uint8_t old = revers_onoff;

  revers_onoff = onoff;

  printf("\e[%um", (onoff) ? 7 : 27);

  return (old);
}
