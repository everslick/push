#include <stddef.h>

#include "str.h"

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
