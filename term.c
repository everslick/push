#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "term.h"

#define POKE(X,Y) (*(unsigned char *)(X))=Y
#define PEEK(X)   (*(unsigned char *)(X))

void togglecase(void) {
#ifdef HAVE_PETSCII
  POKE(0xD018, PEEK(0xD018) ^ 0x02);
#endif
}

#ifdef HAVE_OSD
uint8_t key = 0;
uint8_t osd = 1;

static void hide_osd(lined_t *l) {
  uint8_t i, w = l->cols, x = wherex(), y = wherey();

  for (i=1; i<6; i++) {
    gotoxy(w-7, i); cclear(7);
  }

  gotoxy(x, y);
}

static void show_osd(lined_t *l) {
  uint8_t w = l->cols, h = l->rows, x = wherex(), y = wherey();

  textcolor(COLOR_YELLOW);
  gotoxy(w-7, 1); cprintf("k = %3u", key);
  gotoxy(w-7, 2); cprintf("x = %3u", x);
  gotoxy(w-7, 3); cprintf("y = %3u", y);
  gotoxy(w-7, 4); cprintf("w = %3u", w);
  gotoxy(w-7, 5); cprintf("h = %3u", h);
  textcolor(COLOR_DEFAULT);

  gotoxy(x, y);
}
#endif

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

void term_init() {
#ifdef LINUX
  linux_init();
#endif

#ifdef C64
#endif

#ifdef M65
  togglecase();
#endif

#ifdef ZX
  // 32 column mode
  cputc(1);
  cputc(32);
#endif

  bordercolor(COLOR_BLACK);
  bgcolor(COLOR_BLACK);
  textcolor(COLOR_DEFAULT);

  cursor(1);

  clrscr();
}

void term_fini() {
#ifdef LINUX
  linux_fini();
#endif
}

void term_clear_screen() {
  clrscr();
}

void term_screen_size(uint8_t *cols, uint8_t *rows) {
  screensize(cols, rows);

#ifdef ZX
  // 32 column mode
  *cols >>= 1;
#endif
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

#ifdef HAVE_OSD
  /* Show the OSD if enabled. */
  if (osd) show_osd(l);

  i = (osd && (wherey() > 0) && wherey() < 6) ? 7 : 0;
#else
  i = 0;
#endif

  /* Erase to right */
  cclear((l->cols - wherex()) - i - 1);

  /* Move cursor to original position */
  gotoxy(pos + l->plen, y);
}

uint8_t term_get_key(lined_t *l) {
  uint8_t c = cgetc();

#ifdef HAVE_OSD
  key = c;
#endif

#ifdef ZX
  if (c ==  10) c = TERM_KEY_ENTER;
#endif

#ifdef HAVE_PETSCII
  if (c ==  20) c = TERM_KEY_BACKSPACE;
  if (c == 148) c = TERM_KEY_DELETE; // SHIFT-BACKSPACE
  if (c == 131) c = TERM_KEY_TAB;    // SHIFT-ESCAPE
  if (c ==  94) c = TERM_KEY_CTRL_A; // PG-DOWN
  if (c ==  19) c = TERM_KEY_CTRL_A; // HOME
  if (c == 145) c = TERM_KEY_CTRL_P; // UP
  if (c ==  17) c = TERM_KEY_CTRL_N; // DOWN
  if (c == 157) c = TERM_KEY_CTRL_B; // LEFT
  if (c ==  29) c = TERM_KEY_CTRL_F; // RIGHT
  if (c ==  95) c = TERM_KEY_CTRL_E; // END
#endif

#ifdef C64
  if ((c > 192) && (c <= 192 + 26)) c -= 96;
#endif

#ifdef M65
  if ((c > 96) && (c <= 96 + 26)) c -= 32;
#endif

#ifdef HAVE_PETSCII
  if (c == TERM_KEY_CTRL_R) {
    togglecase();
  }
#endif

#ifdef HAVE_OSD
  if (c == TERM_KEY_CTRL_O) {
    osd ^= 1;

    if (!osd) hide_osd(l);
  }

  if (osd) show_osd(l);
#endif

  return (c);
}

/* Beep, used for completion when there is nothing to complete or when all
 * the choices were already shown. */
void term_make_beep() {
#ifndef LINUX
  bordercolor(COLOR_RED);
  waitvsync(); waitvsync();
  bordercolor(COLOR_BLACK);
#else
  printf("\x7");
  fflush(stdout);
#endif
}
