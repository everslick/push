/* lined.c -- line editing for embedded systems.
 *
 * lined is a fork of the linenoise line editing library originally written
 * by Salvatore Sanfilippo and Pieter Noordhuis. It is stripped down and
 * riddened from all OS specific dependencies to make it usable for small-
 * and/or embedded systems.
 *
 * Additionally the following changes have been made:
 *
 * converted camelcase to underscore syntax
 * added spaces after ',' in parameter lists
 * do not block, while editing the command line
 * allow multiple instances at the same time
 * removed multiline editing mode
 * fixed some inconsistancies in API usage
 * turned switch ... case into if ... else to save ram
 * added macros to put constant strings into flash memory
 * allow local echo to be switched on/off (e.g. for password)
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2016-2021, Clemens Kirchgatterer <clemens at 1541 dot org>
 *
 */

/* linenois.c -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * You can find the latest source code at:
 *
 *   http://github.com/antirez/linenoise
 *
 * Does a number of crazy assumptions that happen to be true in 99.9999% of
 * the 2010 UNIX computers around.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010-2016, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2013, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ------------------------------------------------------------------------
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html
 *
 * Todo list:
 * - Filter bogus Ctrl+<X> combinations.
 * - Win32 support
 *
 * Bloat:
 * - _history_ search like Ctrl+r in readline?
 *
 * List of escape sequences used by this program, we do everything just
 * with three sequences. In order to be so cheap we may have some
 * flickering effect with some slow terminal, but the lesser sequences
 * the more compatible.
 *
 * EL (Erase Line)
 *  Sequence: ESC [ n K
 *  Effect: if n is 0 or missing, clear from cursor to end of line
 *  Effect: if n is 1, clear from beginning of line to cursor
 *  Effect: if n is 2, clear entire line
 *
 * CUF (CUrsor Forward)
 *  Sequence: ESC [ n C
 *  Effect: moves cursor forward n chars
 *
 * CUB (CUrsor Backward)
 *  Sequence: ESC [ n D
 *  Effect: moves cursor backward n chars
 *
 * The following is used to get the terminal width if getting
 * the width with the TIOCGWINSZ ioctl fails
 *
 * DSR (Device Status Report)
 *  Sequence: ESC [ 6 n
 *  Effect: reports the current cusor position as ESC [ n ; m R
 *      where n is the row and m is the column
 *
 * When multi line mode is enabled, we also use an additional escape
 * sequence. However multi line editing is disabled by default.
 *
 * CUU (Cursor Up)
 *  Sequence: ESC [ n A
 *  Effect: moves cursor up of n chars.
 *
 * CUD (Cursor Down)
 *  Sequence: ESC [ n B
 *  Effect: moves cursor down of n chars.
 *
 * When linenoiseClearScreen() is called, two additional escape sequences
 * are used in order to clear the screen and position the cursor at home
 * position.
 *
 * CUP (Cursor position)
 *  Sequence: ESC [ H
 *  Effect: moves the cursor to upper left corner
 *
 * ED (Erase display)
 *  Sequence: ESC [ 2 J
 *  Effect: clear the whole screen
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "term.h"

#include "lined.h"

#ifdef HAVE_HISTORY
static uint8_t history_max = 10; // default history length
static uint8_t history_len = 0;  // current history length
static char  **history = NULL;
#endif

/* Rewrite the currently edited line accordingly to the buffer content,
 * cursor position, and number of columns of the terminal. */
static void refresh_line(lined_t *l) {
  uint8_t len = l->len;
  uint8_t pos = l->pos;
  char *buf = l->buf;

  if (!(l->flags & LINED_ECHO)) return;

  if (l->cols > 0) {
    while ((l->plen + pos) >= l->cols) {
      buf++; len--; pos--;
    }
    while ((l->plen + len) > l->cols) {
      len--;
    }
  }

  l->xpos = pos;

  term_refresh_line(l, buf, len);
}

/* =============================== History ================================ */

#ifdef HAVE_HISTORY

/* Free the global history vector. */
static void history_free(void) {
  if (history) {
    uint8_t j;

    for (j=0; j<history_len; j++) {
      free(history[j]);
      history[j] = NULL;
    }

    history_len = 0;
    free(history);
    history = NULL;
  }
}

#endif

/* ============================= Completion =============================== */

#ifdef HAVE_COMPLETION

/* Free a list of completion option populated by linedAddCompletion(). */
static void reset_completion(lined_t *l) {
  if (l->lc) {
    uint8_t i;

    for (i=0; i<l->lc->len; i++) free(l->lc->cvec[i]);

    free(l->lc->cvec);
  }

  free(l->lc);
  l->lc = NULL;
}

/* Show completion or original buffer */
static void show_completion(lined_t *l) {
  uint8_t i = l->lc->index;

  if (i < l->lc->len) {
    lined_t saved;

    memcpy(&saved, l, sizeof (lined_t));
    strcpy(l->buf, l->lc->cvec[i]);
    l->len = l->pos = strlen(l->buf);

    refresh_line(l);

    strcpy(l->buf, saved.buf);
    l->len = saved.len;
    l->pos = saved.pos;
  } else {
    refresh_line(l);
  }
}

/* This is an helper function for edit() and is called when the
 * user hits <tab> in order to complete the string currently in the
 * input buffer.
 *
 * The state of the editing is encapsulated into the pointed lined_t
 * structure as described in the structure definition. */
static void complete_line(lined_t *l, uint8_t *c) {
  uint8_t i;

  if (!(l->flags & LINED_COMPLETE)) return;

  if (!l->lc && (*c == TERM_KEY_TAB)) {
    *c = TERM_KEY_NONE;

    // fill completion vector
    lined_complete_cb(l);

    if (l->lc) {
      show_completion(l);
    } else {
      term_make_beep();
    }

    return;
  }

  if (!l->lc) return;

  i = l->lc->index;

  if (*c == TERM_KEY_TAB) {
    *c = TERM_KEY_NONE;

    /* Increment completion index */
    i = (i+1) % (l->lc->len+1);
    if (i == l->lc->len) term_make_beep();

    l->lc->index = i;

    show_completion(l);
  } else if (*c == TERM_KEY_ESC) {
    *c = TERM_KEY_NONE;

    /* Re-show original buffer */
    if (i < l->lc->len) refresh_line(l);

    reset_completion(l);
  } else {
    /* Update buffer and return */
    if (i < l->lc->len) {
      strcpy(l->buf, l->lc->cvec[i]);
      l->len = l->pos = strlen(l->buf);
    }

    reset_completion(l);
  }
}

#endif

/* ========================== Line editing ================================ */

/* Insert the character 'c' at cursor current position. */
static void edit_insert(lined_t *l, uint8_t c) {
  if ((c != TERM_KEY_ESC) && (l->len < LINED_LENGTH - 1)) {
    if (l->len != l->pos) {
      memmove(l->buf + l->pos + 1, l->buf + l->pos, l->len - l->pos);
    }

    l->buf[l->pos] = c;
    l->pos++;
    l->len++;
    l->buf[l->len] = 0;

    refresh_line(l);
  }
}

/* Move cursor to the left. */
static void edit_move_left(lined_t *l) {
  if (l->pos > 0) {
    l->pos--;
  }

  refresh_line(l);
}

/* Move cursor to the right. */
static void edit_move_right(lined_t *l) {
  if (l->pos != l->len) {
    l->pos++;
  }

  refresh_line(l);
}

/* Move cursor to the start of the line. */
static void edit_move_home(lined_t *l) {
  if (l->pos != 0) {
    l->pos = 0;
  }

  refresh_line(l);
}

/* Move cursor to the end of the line. */
static void edit_move_end(lined_t *l) {
  if (l->pos != l->len) {
    l->pos = l->len;
  }

  refresh_line(l);
}

#ifdef HAVE_HISTORY

/* Substitute the currently edited line with the next or previous history
 * entry as specified by 'dir'. */
static void edit_history_next(lined_t *l, int8_t dir) {
  int8_t last = (int8_t)history_len - 1;

  if ((l->flags & LINED_HISTORY) && last > 0) {
    /* Update the current history entry before to
     * overwrite it with the next one. */
    free(history[last - l->index]);
    history[last - l->index] = strdup(l->buf);

    /* Show the new entry, NOTE: direction is inverted */
    if (((dir > 0) && (l->index > 0)) || (dir < 0) && (l->index < last)) {
      l->index -= dir;

      strcpy(l->buf, history[last - l->index]);
      l->len = l->pos = strlen(l->buf);

      refresh_line(l);
    }
  }
}

#endif

/* Delete the character at the right of the cursor without altering the
 * cursor position. Basically this is what the DEL keyboard key does. */
static void edit_delete(lined_t *l) {
  if (l->len > 0 && l->pos < l->len) {
    memmove(l->buf + l->pos, l->buf + l->pos + 1, l->len - l->pos - 1);

    l->len--;
    l->buf[l->len] = 0;
  }

  refresh_line(l);
}

/* Backspace implementation. */
static void edit_backspace(lined_t *l) {
  if (l->pos > 0 && l->len > 0) {
    memmove(l->buf + l->pos - 1, l->buf + l->pos, l->len - l->pos);

    l->pos--;
    l->len--;
    l->buf[l->len] = 0;
  }

  refresh_line(l);
}

/* Delete the previous word, maintaining the cursor at the start of the
 * current word. */
static void edit_delete_prev_word(lined_t *l) {
  uint8_t old_pos = l->pos;
  uint8_t diff;

  while (l->pos > 0 && l->buf[l->pos - 1] == ' ') l->pos--;
  while (l->pos > 0 && l->buf[l->pos - 1] != ' ') l->pos--;

  diff = old_pos - l->pos;
  memmove(l->buf + l->pos, l->buf + old_pos, l->len - old_pos + 1);
  l->len -= diff;

  refresh_line(l);
}

/* This function is the core of the line editing capability of lined.
 * It expects the GetKey() function to return every key pressed ASAP
 * or return TERM_KEY_NONE. GetKey() shall never block.
 *
 * The string (in buf) is constantly updated even when using TAB
 * completion.
 *
 * The function returns the code of the last pressed key. */
static uint8_t edit_line(lined_t *l, uint8_t key) {
  l->key = key;

#ifdef HAVE_COMPLETION
  /* Handle autocompletion. */
  complete_line(l, &key);
#endif

  if (key == TERM_KEY_ENTER) {
#ifdef HAVE_HISTORY
    if (history_len > 0) {
      free(history[--history_len]);
    }
#endif

#ifdef HAVE_HINTS
    if (l->flags & LINED_HINTS) {
      /* Force a refresh without hints to leave the previous
       * line as the user typed it after a newline. */
      l->flags &= ~LINED_HINTS;
      refresh_line(l);
      l->flags |= LINED_HINTS;
    }
#endif

#ifdef HAVE_HISTORY
    lined_history_add(l->buf);
#endif

    return (TERM_KEY_ENTER);
  } else if (key == TERM_KEY_CTRL_C) {
    return (TERM_KEY_CTRL_C);
  } else if (key == TERM_KEY_BACKSPACE) {
    edit_backspace(l);
  } else if (key == TERM_KEY_DELETE) {
    edit_delete(l);
  } else if (key == TERM_KEY_CTRL_D) {
    /* remove char at right of cursor, or if the
     * line is empty, act as end-of-file. */
    if (l->len > 0) {
      edit_delete(l);
    } else {
#ifdef HAVE_HISTORY
      if (history_len > 0) {
        free(history[--history_len]);
      }
#endif

      return (TERM_KEY_CTRL_D);
    }
  } else if (key == TERM_KEY_CTRL_T) {
    /* swaps current character with previous. */
    if (l->pos > 0 && l->pos < l->len) {
      uint8_t aux = l->buf[l->pos-1];

      l->buf[l->pos-1] = l->buf[l->pos];
      l->buf[l->pos] = aux;
      if (l->pos != l->len-1) l->pos++;

      refresh_line(l);
    }
  } else if (key == TERM_KEY_CTRL_B) {
    edit_move_left(l);
  } else if (key == TERM_KEY_CTRL_F) {
    edit_move_right(l);
#ifdef HAVE_HISTORY
  } else if (key == TERM_KEY_CTRL_P) {
    edit_history_next(l, -1);
  } else if (key == TERM_KEY_CTRL_N) {
    edit_history_next(l,  1);
#endif
  } else if (key == TERM_KEY_CTRL_U) {
    /* delete the whole line. */
    l->buf[0] = 0;
    l->pos = l->len = 0;
    refresh_line(l);
  } else if (key == TERM_KEY_CTRL_K) {
    /* delete from current to end of line. */
    l->buf[l->pos] = 0;
    l->len = l->pos;
    refresh_line(l);
  } else if (key == TERM_KEY_CTRL_A) {
    /* go to the start of the line */
    edit_move_home(l);
  } else if (key == TERM_KEY_CTRL_E) {
    /* go to the end of the line */
    edit_move_end(l);
  } else if (key == TERM_KEY_CTRL_L) {
    /* clear screen */
    term_clear_screen();
    refresh_line(l);
  } else if (key == TERM_KEY_CTRL_W) {
    /* delete previous word */
    edit_delete_prev_word(l);
  } else {
    if (key >= 32 && key < 127) {
      edit_insert(l, key);
    }
  }

  return (TERM_KEY_NONE);
}

/* ============================= lined API ================================ */

/* The high level function that creates a new lined context. */
lined_t *lined_init(void) {
  lined_t *l = (lined_t *)malloc(sizeof (lined_t));

  if (!l) return (NULL);

  /* Initialize the lined state that we pass to functions implementing
   * specific editing functionalities. */
  memset(l, 0, sizeof (lined_t));
  l->flags = 0x0f;

  term_screen_size(&l->cols, &l->rows);

  return (l);
}

void lined_reset(lined_t *l, uint8_t flags) {
#ifdef HAVE_HISTORY
  /* The latest history entry is always our current buffer, that
   * initially is just an empty string. */
  lined_history_add("");
  l->index = 0;
#endif

#ifdef HAVE_COMPLETION
  reset_completion(l);
#endif

  l->buf[0] = 0;
  l->pos = 0;
  l->len = 0;
  l->key = 0;

  // show the prompt, even when ECHO is off
  l->flags = flags | LINED_ECHO;
  refresh_line(l);
  l->flags = flags;
}

void lined_prompt(lined_t *l, const char *prompt) {
  l->prompt = prompt;
  l->plen   = (uint8_t)strlen(l->prompt);
}

void lined_resize(lined_t *l, uint8_t w, uint8_t h) {
  l->cols = w;
  l->rows = h;
}

char *lined_line(lined_t *l) {
  return (l->buf);
}

void lined_edit(lined_t *l, uint8_t key) {
  edit_line(l, key);
}

void lined_fini(lined_t *l) {
#ifdef HAVE_COMPLETION
  reset_completion(l);
#endif
#ifdef HAVE_HISTORY
  //history_free();
#endif

  free(l);
}

/* ============================= Completion =============================== */

/* This function is used by the callback function registered by the user
 * in order to add completion options given the input string when the
 * user hit <tab>. */
void lined_completion_add(lined_t *l, const char *str) {
#ifdef HAVE_COMPLETION
  uint8_t len = strlen(str);
  char *copy, **cvec;

  if (!l->lc) {
    l->lc = (completion_t *)malloc(sizeof (completion_t));

    l->lc->len   = 0;
    l->lc->index = 0;
    l->lc->cvec  = NULL;
  }

  copy = (char *)malloc(len + 1);

  if (copy == NULL) return;

  memcpy(copy, str, len + 1);
  cvec = (char **)realloc(l->lc->cvec, sizeof (char *) * (l->lc->len + 1));

  if (cvec == NULL) {
    free(copy);
    return;
  }

  l->lc->cvec = cvec;
  l->lc->cvec[l->lc->len++] = copy;
#endif
}

/* =============================== History ================================ */


/* This is the API call to add a new entry in the lined history.
 * It uses a fixed array of char pointers that are shifted (memmoved)
 * when the history max length is reached in order to remove the older
 * entry and make room for the new one, so it is not exactly suitable
 * for huge histories, but will work well for a few hundred of entries. */
void lined_history_add(const char *line) {
#ifdef HAVE_HISTORY
  char *copy = NULL;

  if (history_max == 0) return;

  /* Initialization on first call. */
  if (history == NULL) {
    history = (char **)malloc(sizeof (char *) * history_max);

    if (history == NULL) return;

    memset(history, 0, (sizeof (char *) * history_max));
  }

  /* Don't add duplicates. */
  if (history_len && !strcmp(history[history_len-1], line)) return;

  /* Add an heap allocated copy of the line in the history.
   * If we reached the max length, remove the older line. */

  if (!(copy = strdup(line))) return;

  if (history_len == history_max) {
    free(history[0]);
    memmove(history, history+1, sizeof (char *) * (history_max-1));
    history_len--;
  }

  history[history_len++] = copy;
#endif
}

/* Set the maximum length for the history. This function can be called even
 * if there is already some history, the function will make sure to retain
 * just the latest 'len' elements if the new history length value is smaller
 * than the amount of items already inside the history. */
void lined_history_len(uint8_t len) {
#ifdef HAVE_HISTORY
  if (len < 1) return;

  if (history) {
    uint8_t tocopy = history_len;

    char **n = (char **)malloc(sizeof (char *) * len);

    if (n == NULL) return;

    /* If we can't copy everything, free the elements we'll not use. */
    if (len < tocopy) {
      uint8_t j;

      for (j=0; j<tocopy-len; j++) {
        free(history[j]);
      }

      tocopy = len;
    }

    memset(n, 0, sizeof (char *) * len);
    memcpy(n, history + (history_len - tocopy), sizeof (char *) * tocopy);
    free(history);
    history = n;
  }

  history_max = len;
  if (history_len > history_max) {
    history_len = history_max;
  }
#endif
}
