#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define POKE(X,Y) (*(unsigned char *)(X))=Y
#define PEEK(X)   (*(unsigned char *)(X))

uint8_t cgetc(void) {
  uint8_t k = 0;

  while (!k) {
    k = PEEK(0xD610);
  }

  while (PEEK(0xD610)) POKE(0xD610,0);

  return (k);
}

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
      if (*s && (*s == '"')) { s++; quote ^= 1; }

      // start next arg
      *argv++ = s;

      if (++argc == args) {
        return (argc);
      }
    }

    if (*s == '"') {
      *s++ = 0; // remove and skip
      if (*s == '"') first = 1;
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

  clrscr();
  bgcolor(0);
  textcolor(1);
  list(xargc, xargv);

  uint8_t k = 0;

  if ((k=cgetc())) {
    printf("key = '%i'\r\n", (int)k);
  }

  return (0);
}
