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

char *strchr(const char *s, char c) {
  while (*s != c) {
    if (!*s++) {
      return (NULL);
    }
  }

  return ((char *)s);
}

char *strrchr(const char *s, char c) {
  const char *found = NULL, *p;

  if (c == 0) return (strchr(s, '\0'));

  while ((p = strchr(s, c)) != NULL) {
    found = p;
    s = p + 1;
  }

  return ((char *)found);
}

char *strtok(char *s, const char *delim) {
  static char *buf;
  const char *d;
  char *ret, *b;

  if (s != NULL) buf = s;
  if (buf[0] == '\0') return (NULL);

  ret = buf;

  for (b = buf; *b !='\0'; b++) {
    for (d = delim; *d != '\0'; d++) {
      if (*b == *d) {
        *b = '\0';
        buf = b + 1;

        if (b == ret) {
          ret++; continue;
        }

        return (ret);
      }
    }
  }

  return (ret);
}
