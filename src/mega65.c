#include <ctype.h>

#include "mega65.h"

#define POKE(X,Y) (*(unsigned char *)(X))=Y
#define PEEK(X)   (*(unsigned char *)(X))

static uint8_t revers_onoff = 0;

void waitvsync(void) {
  int c = 500;

  while (c--);
}

uint8_t revers(uint8_t onoff) {
  uint8_t old = revers_onoff;

  revers_onoff = onoff;

  cputc((onoff) ? 18 : 148);

  return (old);
}

uint8_t cgetc(void) {
  uint8_t k = 0;

  while (!k) {
    k = PEEK(0xD610);
  }

  POKE(0xD610, 0);

  return (k);
}

int sleep(int seconds) {
  return (0);
}
