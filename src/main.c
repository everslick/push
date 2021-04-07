#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "lined.h"
#include "term.h"
#include "cli.h"

#include "push.h"

char scratch[sizeof (scratch)];

#ifdef POSIX
static uint8_t reset_once_after_startup = 1;
#endif

int main(void) {
  lined_t *lined;
  uint8_t logout;
  uint8_t restart;

#ifndef KICKC
loop:
#endif

  lined   = NULL;
  logout  = 0;
  restart = 0;

#ifdef POSIX
  // workaround for mangled xterm after startup
  if (reset_once_after_startup) {
    reset_once_after_startup = 0;

    restart = 1;
    logout  = 1;
  }
#endif

  term_init();

  if (!(lined = lined_init())) {
    printf("push: out of memory" LF); return (1);
  }

  textcolor(COLOR_CYAN);
  printf(
    "       ____  __  _______ __  __"  LF
    "      / __ \\/ / / / ___// / / /" LF
    "     / /_/ / / / /\\__ \\/ /_/ /" LF
    "    / ____/ /_/ /___/ / __  /"    LF
    "   / /    \\____//____/_/ / /"    LF
    "  /_/ petite un*x shell /_/"      LF
  );
  textcolor(COLOR_DEFAULT);
  printf(LF);

  lined_prompt(lined, "push:$ ");
  lined_reset(lined, LINED_HISTORY | LINED_COMPLETE | LINED_HINTS | LINED_ECHO);

  while (!logout) {
    uint8_t key = term_get_key(lined);

    lined_edit(lined, key);

    if (key == TERM_KEY_ENTER) {
      char *cmd = lined_line(lined);
      uint8_t ret;

      printf(LF);

      ret = cli_exec(cmd);

      if (ret == 1) {
        logout = 1;
      } else if (ret == 2) {
        restart = 1; // reset
        logout = 1;
      }

      lined_reset(lined, LINED_HISTORY | LINED_COMPLETE | LINED_HINTS | LINED_ECHO);
    } else if (key == TERM_KEY_CTRL_C) {
      printf("break" LF);
      logout = 1;
    } else if (key == TERM_KEY_CTRL_D) {
      printf("exit" LF);
      logout = 1;
    }
  }

  lined_fini(lined);

  term_fini();

#ifndef KICKC
  if (restart) goto loop;
#endif

  return (0);
}
