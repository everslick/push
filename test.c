#include <string.h>
#include <stdint.h>
#include <stdio.h>

static const char *test = "   echo     this is a  '   foo  ' test  ";

static uint8_t parse(char *cmd, char **argv, uint8_t args) {
  uint8_t i, argc = 0, quote = 0, first = 1;
  char *s = cmd, *t = s + strlen(s) - 1;

  while ((t >= s) && (*t && (*t == ' '))) {
    *t-- = 0; // trim end
  }

  for (i=0; i<args; i++) {
    argv[i] = 0;
  }

  while (*s) {
    if (first) {
      first = 0;

      // skip whitespace
      while (*s && (*s == ' ')) s++;

      // skip quotation mark
      if (*s && (*s == '\'')) { s++; quote ^= 1; }

      // start next arg
      *argv++ = s;

      if (++argc == args) {
        return (argc);
      }
    }

    if (*s == '\'') {
      *s++ = 0; // remove and skip
      if (*s == '\'') first = 1;
      quote ^= 1;
    } else if ((!quote) && (*s == ' ')) {
      *s++ = 0; // remove and skip
      first = 1;
    } else {
      s++;
    }
  }

  return (argc);
}

static void list(uint8_t argc, char **argv) {
  uint8_t i;

  printf("\n");
  for (i=0; i<argc; i++) {
    printf("argv[%i]='%s'\n", (int)i, argv[i]);
  }
}

int main() {
  char cmd[80], *xargv[8];
  uint8_t xargc;

  strcpy(cmd, test);

  xargc = parse(cmd, xargv, 8);

  list(xargc, xargv);

  return (0);
}
