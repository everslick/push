#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "term.h"

#define OSD_W 7
#define OSD_H 8

#define POKE(X,Y) (*(unsigned char *)(X))=Y
#define PEEK(X)   (*(unsigned char *)(X))

static const char *keys = NULL;

static void togglecase(void) {
#ifdef HAVE_PETSCII
  POKE(0xD018, PEEK(0xD018) ^ 0x02);
#endif
}

static void clear(uint8_t length) {
  uint8_t i;

  for (i=0; i<length; i++) cputc(' ');
}

#ifdef HAVE_OSD
static uint8_t osd = 0;

static void hide_osd(lined_t *l) {
  uint8_t i, w = l->cols, x = wherex(), y = wherey();

  for (i=0; i<OSD_H; i++) {
    gotoxy(w-OSD_W, i); clear(OSD_W);
  }

  gotoxy(x, y);
}

static void show_osd(lined_t *l) {
  uint8_t w = l->cols, h = l->rows, x = wherex(), y = wherey();

#ifdef ZX
  textbackground(COLOR_YELLOW);
  textcolor(COLOR_BLACK);
#else
  textcolor(COLOR_YELLOW);
  revers(1);
#endif

  gotoxy(w-OSD_W, 0); cprintf("       ");
  gotoxy(w-OSD_W, 1); cprintf(" k:%3u ", l->key);
  gotoxy(w-OSD_W, 2); cprintf(" p:%3u ", l->pos);
  gotoxy(w-OSD_W, 3); cprintf(" x:%3u ", l->xpos + l->plen);
  gotoxy(w-OSD_W, 4); cprintf(" y:%3u ", y);
  gotoxy(w-OSD_W, 5); cprintf(" w:%3u ", w);
  gotoxy(w-OSD_W, 6); cprintf(" h:%3u ", h);
  gotoxy(w-OSD_W, 7); cprintf("       ");

#ifdef ZX
  textbackground(COLOR_BLACK);
#else
  revers(0);
#endif

  textcolor(COLOR_DEFAULT);

  gotoxy(x, y);
}
#endif // HAVE_OSD

#ifdef HAVE_HINTS
static void show_hint(lined_t *l) {
  if ((l->flags & LINED_HINTS) && (l->plen + l->len < l->cols)) {
    const char *hint = term_hint_cb(l);

    if (hint) {
      uint8_t max = l->cols - l->plen - l->len - 1; // one extra space
      uint8_t i, len = (uint8_t)strlen(hint);

#ifdef HAVE_OSD
      if (osd && (wherey() < OSD_H)) max -= OSD_W;
#endif

      cputc(' ');

      if (len > max) len = max;

      textcolor(COLOR_BLUE);
      for (i=0; i<len; i++) cputc(hint[i]);
    }
  }
}
#endif

void term_init(void) {
#ifdef POSIX
  posix_init();
#endif

#ifdef M65
  togglecase();
#endif

  bordercolor(COLOR_BLACK);
  bgcolor(COLOR_BLACK);
  textcolor(COLOR_DEFAULT);

#ifdef HAVE_SWCURSOR
  cursor(0);
#else
  cursor(1);
#endif

  clrscr();
}

void term_fini(void) {
#ifdef POSIX
  posix_fini();
#endif
}

void term_clear_screen(void) {
  clrscr();
}

void term_screen_size(uint8_t *cols, uint8_t *rows) {
#ifdef ZX
  screensize((uint *)cols, (uint *)rows);
#else
  screensize(cols, rows);
#endif
}

/* Rewrite the currently edited line accordingly to the buffer content,
 * cursor position, and number of columns of the terminal. */
void term_refresh_line(lined_t *l, char *buf, uint8_t len) {
  uint8_t i, x, y = wherey();

#ifdef HAVE_OSD
  uint8_t max = l->cols - l->plen;

  if (osd && (y < OSD_H)) max -= OSD_W;
  if (len > max) len = max;
#endif

  /* Cursor to left edge */
  gotoxy(0, y);

  /* Write the prompt */
  textcolor(COLOR_RED);
  for (i=0; i<l->plen; i++) cputc(l->prompt[i]);

  /* Write the current buffer content */
  textcolor(COLOR_WHITE);
  for (i=0; i<len; i++) cputc(buf[i]);

#ifdef HAVE_HINTS
  /* Write the hint if any */
  show_hint(l);
#endif

  /* Erase to right edge */
  x = wherex();
  if (x < l->cols) {
    len = l->cols - x;
#ifdef HAVE_OSD
    if (osd && (y < OSD_H)) len -= OSD_W;
#endif
    clear(len);
  }

#ifdef HAVE_OSD
  /* Show the OSD if enabled. */
  if (osd) show_osd(l);
#endif

  /* Calculate screen X position. */
  x = l->plen + l->xpos;

  /* Move cursor to original position */
  gotoxy(x, y);

#ifdef HAVE_SWCURSOR
  /* Draw software cursor. */
  if (l->key != TERM_KEY_ENTER) {
    textbackground(COLOR_CYAN);
    textcolor(COLOR_WHITE);
    cputc((l->pos < l->len) ? l->buf[l->pos] : ' ');
    gotoxy(x, y);
  }

  textbackground(COLOR_BLACK);
#endif

  /* Reset text color */
  textcolor(COLOR_DEFAULT);
}

uint8_t term_get_key(lined_t *l) {
  uint8_t c = keys ? *keys++ : cgetc();

  // end of input stream
  if (keys && (*keys == 0)) {
    keys = NULL;
  }

  if ((c == 10) || (c == 13)) c = TERM_KEY_ENTER;

#ifdef ATARI
  if (c == 126) c = TERM_KEY_BACKSPACE;
  if (c == 127) c = TERM_KEY_TAB;
  if (c == 253) c = TERM_KEY_BELL;
  if (c == 254) c = TERM_KEY_DELETE;
  if (c == 155) c = TERM_KEY_ENTER;
  if (c == 156) c = TERM_KEY_CTRL_U; // DEL LINE
  if (c == 125) c = TERM_KEY_CTRL_L; // CLS
  if (c ==  28) c = TERM_KEY_CTRL_P; // UP
  if (c ==  29) c = TERM_KEY_CTRL_N; // DOWN
  if (c ==  30) c = TERM_KEY_CTRL_B; // LEFT
  if (c ==  31) c = TERM_KEY_CTRL_F; // RIGHT
#endif

#ifdef ZX
  if (c ==  12) c = TERM_KEY_BACKSPACE;
  if (c ==  14) c = TERM_KEY_TAB;
  if (c == 205) c = TERM_KEY_CTRL_D;
  if (c == 199) c = TERM_KEY_CTRL_C; // TERM_KEY_CTRL_Q; // BREAK
  if (c == 201) c = TERM_KEY_CTRL_O; // TERM_KEY_CTRL_W; // OSD

  if (c == 226) c = TERM_KEY_CTRL_A; // HOME
  if (c == 200) c = TERM_KEY_CTRL_E; // END

  if (c == 195) c = TERM_KEY_DELETE; // TERM_KEY_CTRL_S; // DELETE
  if (c == 198) c = TERM_KEY_CTRL_B; // TERM_KEY_CTRL_Y; // LEFT
  if (c == 172) c = TERM_KEY_CTRL_F; // TERM_KEY_CTRL_I; // RIGHT
  if (c == 197) c = TERM_KEY_CTRL_P; // TERM_KEY_CTRL_U; // UP
  if (c == 204) c = TERM_KEY_CTRL_P; // TERM_KEY_CTRL_F; // UP
  if (c == 203) c = TERM_KEY_CTRL_N; // TERM_KEY_CTRL_G; // DOWN
#endif

#ifdef HAVE_PETSCII
  if (c ==  20) c = TERM_KEY_BACKSPACE;
  if (c == 148) c = TERM_KEY_DELETE; // SHIFT-BACKSPACE
  if (c == 131) c = TERM_KEY_TAB;    // SHIFT-ESCAPE
  if (c ==  94) c = TERM_KEY_TAB;    // SHIFT-TILDE
  if (c ==  26) c = TERM_KEY_TAB;    // CTRL-Z
  if (c ==  19) c = TERM_KEY_CTRL_A; // HOME
  if (c ==  95) c = TERM_KEY_CTRL_E; // END
  if (c == 145) c = TERM_KEY_CTRL_P; // UP
  if (c ==  17) c = TERM_KEY_CTRL_N; // DOWN
  if (c == 157) c = TERM_KEY_CTRL_B; // LEFT
  if (c ==  29) c = TERM_KEY_CTRL_F; // RIGHT
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

    if (!osd) {
      hide_osd(l);
    }
  }

  if (osd) {
    show_osd(l);
  }
#endif

#ifdef HAVE_SWCURSOR
  if ((c == TERM_KEY_ENTER) || (c == TERM_KEY_BACKSPACE)) {
    cputc(' '); // delete software cursor
    gotoxy(l->plen + l->xpos, wherey());
  }
#endif

  return (c);
}

/* Beep, used for completion when there is nothing to complete or when all
 * the choices were already shown. */
void term_make_beep(void) {
#ifndef POSIX
  bordercolor(COLOR_RED);
  waitvsync(); waitvsync();
  bordercolor(COLOR_BLACK);
#else
  cputs("\x7");
#endif
}

void term_push_keys(const char *str) {
  keys = str;
}
