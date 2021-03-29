#include "mega65.h"

int8_t strncmp(const char *s1, const char *s2, uint8_t n) {
  uint8_t i;

  for (i=0; i<n; i++) {
    if (s1[i] != s2[i]) {
      return ((s1[i]>s2[i]) ? 1 : -1);
    }

    if (!s1[i] || !s2[i]) break;
  }

  return (0);
}

int8_t strcmp(const char *s1, const char *s2) {
  return (strncmp(s1, s2, 255));
}

char cgetc(void) {
  return (0);
}

void cclear(uint8_t length) {
  uint8_t i;

  for (i=0; i<length; i++) cputc(' ');
}

void waitvsync() {
}
