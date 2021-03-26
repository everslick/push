#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "term.h"

#ifdef VT100

//
// LINUX
//

uint8_t screen_init(void) {
  return (vt100_init());
}

void screen_fini(void) {
  vt100_fini();
}

static void make_beep() {
  printf("\x7");
  fflush(stdout);
}

#endif

#if defined(C64) || defined(M65)

//
// C64, MEGA65, ...
//

static uint8_t screen_init() {
  cputc(8);
 
  return (1);
}

static void screen_fini() {
}

static void make_beep() {
  bordercolor(COLOR_RED);
  waitvsync();
  waitvsync();
  bordercolor(COLOR_BLACK);
}

#endif

#ifdef ZX

//
// ZX Spectrum
//

static uint8_t screen_init() {
  return (1);
}

static void screen_fini() {
}

static void make_beep() {
  bordercolor(COLOR_RED);
  bordercolor(COLOR_BLACK);
}

#endif

//
// COMMON
//

static void show_hint(lined_t *l) {
#ifdef HAVE_HINTS
  if ((l->flags & LINED_HINTS) && (l->plen + l->len < l->cols)) {
    const char *hint = term_hint_cb(l);

    if (hint) {
      uint8_t i, len = (uint8_t)strlen(hint);
      uint8_t maxlen = l->cols - (l->plen + l->len);

      cputc(' ');

      if (len > maxlen) len = maxlen;

      textcolor(COLOR_BLUE);
      for (i=0; i<len; i++) cputc(hint[i]);
      textcolor(COLOR_DEFAULT);
    }
  }
#endif
}

//
// API
//

uint8_t term_init() {
  uint8_t ok = screen_init();

  if (ok) {
    bordercolor(COLOR_BLACK);
    bgcolor(COLOR_BLACK);
    textcolor(COLOR_DEFAULT);

    cursor(1);
    clrscr();
  }

  return (ok);
}

void term_fini() {
  screen_fini();
}

void term_clear_screen() {
  clrscr();
}

void term_get_screen_size(uint8_t *cols, uint8_t *rows) {
  screensize(cols, rows);
}

/* Rewrite the currently edited line accordingly to the buffer content,
 * cursor position, and number of columns of the terminal. */
void term_refresh_line(lined_t *l, char *buf, uint8_t len, uint8_t pos) {
  uint8_t i, y = wherey();

  /* Cursor to left edge */
  gotoxy(0, y);

  /* Write the prompt */
  textcolor(COLOR_RED);
  cputs(l->prompt);
  textcolor(COLOR_DEFAULT);

  /* Write the current buffer content */
  textcolor(COLOR_WHITE);
  for (i=0; i<len; i++) cputc(buf[i]);
  textcolor(COLOR_DEFAULT);

  /* Write the hint if any */
  show_hint(l);

  /* Erase to right */
  cclear(l->cols - wherex() - 1);

  /* Move cursor to original position */
  gotoxy(pos + l->plen, y);
}

uint8_t term_get_key() {
  uint8_t c = cgetc();

  if (c ==  20) c = TERM_KEY_BACKSPACE;
  if (c == 148) c = TERM_KEY_DELETE; // SHIFT-BACKSPACE
  if (c == 131) c = TERM_KEY_TAB;    // SHIFT-ESCAPE
  if (c ==  94) c = TERM_KEY_HOME;   // PG-DOWN
  if (c ==  19) c = TERM_KEY_HOME;
  if (c == 145) c = TERM_KEY_UP;
  if (c ==  17) c = TERM_KEY_DOWN;
  if (c == 157) c = TERM_KEY_LEFT;
  if (c ==  29) c = TERM_KEY_RIGHT;
  if (c ==  95) c = TERM_KEY_END;

#ifndef VT100
  if ((c > 96) && (c < 123)) c -= 96;
#endif

  return (c);
}

/* Beep, used for completion when there is nothing to complete or when all
 * the choices were already shown. */
void term_make_beep() {
  make_beep();
}
