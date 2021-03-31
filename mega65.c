#include <ctype.h>

#include "mega65.h"

#define POKE(X,Y) (*(unsigned char *)(X))=Y
#define PEEK(X)   (*(unsigned char *)(X))

int8_t strncmp(const char *s1, const char *s2, uint8_t n) {
  uint8_t i;

  for (i=0; i<n; i++) {
    char c1 = toupper(s1[i]);
    char c2 = toupper(s2[i]);

    if (c1 != c2) {
      return ((c1>c2) ? 1 : -1);
    }

    if (!c1 || !c2) break;
  }

  return (0);
}

int8_t strcmp(const char *s1, const char *s2) {
  return (strncmp(s1, s2, 255));
}

uint8_t cgetc(void) {
  uint8_t k = 0;

  while (!k) {
    k = PEEK(0xD610);
  }

  POKE(0xD610, 0);

  return (k);
}
