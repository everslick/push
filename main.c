#include <stdint.h>
#include <stdlib.h>

#include "lined.h"
#include "term.h"
#include "cli.h"

int main() {
  lined_t *lined = lined_init();

  if (!lined) {
    cprintf("push: out of memory\r\n"); return (1);
  }

  if (!term_init()) {
    cprintf("push: cant't init terminal\r\n"); return (1);
  }

  cprintf(
    "       ____  __  _______ __  __\r\n"
    "      / __ \\/ / / / ___// / / /\r\n"
    "     / /_/ / / / /\\__ \\/ /_/ /\r\n"
    "    / ____/ /_/ /___/ / __  /\r\n"
    "   / /    \\____//____/_/ / /\r\n"
    "  /_/ petite un*x shell /_/\r\n\r\n"
  );

  lined_prompt(lined, "push:$ ");
  lined_reset(lined, LINED_HISTORY | LINED_COMPLETE | LINED_HINTS | LINED_ECHO);

  while (1) {
    uint8_t key = lined_poll(lined);

    if (key == TERM_KEY_ENTER) {
      char *cmd = lined_line(lined);

      cprintf("\r\n");

      if (cli_exec(cmd)) {
        break; // 'exit' or 'logout'
      }

      lined_reset(lined, LINED_HISTORY | LINED_COMPLETE | LINED_HINTS | LINED_ECHO);
    } else if (key == TERM_KEY_CTRL_C) {
      cprintf("break\r\n");
      break;
    } else if (key == TERM_KEY_CTRL_D) {
      cprintf("exit\r\n");
      break;
    }
  }

  term_fini();

  lined_fini(lined);

  return (0);
}
