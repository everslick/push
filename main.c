#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "lined.h"
#include "term.h"
#include "cli.h"

int main(void) {
  lined_t *lined;
  uint8_t reset;

#ifndef KICKC
restart:
#endif

  lined = NULL;
  reset = 0;

  term_init();

  if (!(lined = lined_init())) {
    printf("push: out of memory" LF); return (1);
  }

  lined_prompt(lined, "push:$ ");

  printf(
    "       ____  __  _______ __  __"  LF
    "      / __ \\/ / / / ___// / / /" LF
    "     / /_/ / / / /\\__ \\/ /_/ /" LF
    "    / ____/ /_/ /___/ / __  /"    LF
    "   / /    \\____//____/_/ / /"    LF
    "  /_/ petite un*x shell /_/"      LF
  );

  printf(LF);
  textcolor(COLOR_YELLOW);
  //printf("w = %d" LF, lined->cols);
  //printf("h = %d" LF, lined->rows);
  textcolor(COLOR_DEFAULT);
  printf(LF);

  lined_reset(lined, LINED_HISTORY | LINED_COMPLETE | LINED_HINTS | LINED_ECHO);

  while (1) {
    uint8_t key = term_get_key(lined);

    lined_edit(lined, key);

    if (key == TERM_KEY_ENTER) {
      char *cmd = lined_line(lined);
      uint8_t ret;

      printf(LF);

      ret = cli_exec(cmd);

      if (ret == 1) {
        break; // 'exit' or 'logout'
      } else if (ret == 2) {
        reset = 1; // reset
        break;
      }

      lined_reset(lined, LINED_HISTORY | LINED_COMPLETE | LINED_HINTS | LINED_ECHO);
    } else if (key == TERM_KEY_CTRL_C) {
      printf("break" LF);
      break;
    } else if (key == TERM_KEY_CTRL_D) {
      printf("exit" LF);
      break;
    }
  }

  lined_fini(lined);

  term_fini();

#ifndef KICKC
  if (reset) goto restart;
#endif

  return (0);
}
