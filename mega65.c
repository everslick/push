#include "mega65.h"

void waitvsync() {
}

char cgetc(void) {
  return (0);
}

void cclear(uint8_t length) {
  uint8_t i;

  for (i=0; i<length; i++) cputc(' ');
}
