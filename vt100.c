#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "vt100.h"

// TERMINAL ATTRIBUTES
#define TERM_RESET           0
#define TERM_BRIGHT          1
#define TERM_DIM             2
#define TERM_UNDERLINE       3
#define TERM_BLINK           4
#define TERM_REVERSE         7
#define TERM_HIDDEN          8

static struct termios initial_settings;

static uint8_t cursor_onoff = 0;

static uint8_t color_fg = 0;
static uint8_t color_bg = 0;
static uint8_t color_bd = 0;

static uint8_t cursor_x = 1;
static uint8_t cursor_y = 1;
static uint8_t screen_w = 0;
static uint8_t screen_h = 0;

static void set_cursor_pos(uint8_t col, uint8_t row) {
  printf("\e[%i;%iH", row+1, col+1);
  printf("\e[%i;%if", row+1, col+1);
  fflush(stdout);
}

static uint8_t get_cursor_pos(uint8_t *col, uint8_t *row) {
  char buf[16], *p = buf;
  uint8_t timeout = 1;
  uint8_t pos = 0;
  int c, r;

  // request cursor location
  printf("\e[6n");
  fflush(stdout);

  // wait for response to arrive
  for (uint8_t i=0; i<50; i++) {
    char c;

    if (read(0, &c, 1)) {
      // read until 'R'
      if (c == 'R') {
        timeout = 0;
        break;
      }

      buf[pos++] = c;
    }
  }

  // check for timeout
  if (timeout) {
    return (0);
  }

  // sometimes the first byte is '\0', so skip any '\0' bytes
  for (uint8_t i=0; i<pos; i++) {
    if (*p == 0) p++; else break;
  }

  // check response
  if ((p[0] != 27) || (p[1] != '[')) {
    return (0);
  }

  // parse it
  if (sscanf(p+2, "%i;%i", &r, &c) != 2) {
    return (0);
  }

  *row = r - 1;
  *col = c - 1;

  return (1);
}

static uint8_t get_screen_size(uint8_t *cols, uint8_t *rows) {
  uint8_t col, row;

  // get the initial position
  if (!get_cursor_pos(&col, &row)) return (0);
  // go to bottom-right corner
  set_cursor_pos(255, 255);
  // get current position
  if (!get_cursor_pos(cols, rows)) return (0);
  // restore initial position
  set_cursor_pos(col, row);

  return (1);
}

uint8_t vt100_init() {
	struct termios new_settings;
                                                                                
	if (tcgetattr(0, &initial_settings) < 0) return (0);

	new_settings = initial_settings;
	new_settings.c_lflag &= ~(ICANON | ECHO | ISIG); // Ctrl-C is disabled
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 1;

	tcsetattr(0, TCSANOW, &new_settings);

  if (!get_screen_size(&screen_w, &screen_h)) return (0);

  cursor(cursor_onoff);

  return (1);
}

void vt100_fini() {
	tcsetattr(0, TCSANOW, &initial_settings);
}

void clrscr(void) {
  printf("\e[H\e[J");
  gotoxy(0, 0);
}

void gotoxy(uint8_t x, uint8_t y) {
  cursor_x = x;
  cursor_y = y;

  set_cursor_pos(x, y);
}

uint8_t wherex(void) {
  return (cursor_x);
}

uint8_t wherey(void) {
  return (cursor_y);
}

void cputs(const char *s) {
  int len = printf("%s", s);
  fflush(stdout);

  for (int i=0; i<len; i++) {

         if (s[i] == '\r') cursor_x = 0;
    else if (s[i] == '\n') cursor_y++;
    else                   cursor_x++;

    if (cursor_x >= screen_w) {
      cursor_x = 0;
      cursor_y++;
    }
  }

  if (cursor_y >= screen_h) {
    cursor_y = screen_h - 1;
  }
}

void cputc(char c) {
  char s[] = { c, '\0' };

  cputs(s);
}

char cgetc(void) {
  uint8_t seq[4], c = 0;

	if (!read(0, &c, 1)) {
    return (0);
  }

  if (c ==  10) return (13); // LINEFEED
  if (c ==  13) return (13); // RETURN
  if (c == 127) return (20); // BACKSPACE
  if (c ==  27) { // escape sequence

    // read the next two bytes representing the escape sequence.
    // use two calls to handle slow terminals returning the two
    // chars at different times.

	  if (!read(0, seq+0, 1)) return (27);
	  if (!read(0, seq+1, 1)) return (0);

    // ESC [ sequences
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        // extended escape, read additional byte
	      if (!read(0, seq+2, 1)) return (0);
        if (seq[2] == '~') {
          if (seq[1] == '2') return (43);  // INSERT
          if (seq[1] == '3') return (127); // DELETE
          if (seq[1] == '5') return (145); // PG-UP
          if (seq[1] == '6') return (17);  // PG_DOWN
        } else {
	        if (!read(0, seq+3, 1)) return (0);
        }
      } else {
        if (seq[1] == 'A') return (145); // UP
        if (seq[1] == 'B') return (17);  // DOWN
        if (seq[1] == 'C') return (29);  // RIGHT
        if (seq[1] == 'D') return (157); // LEFT
        if (seq[1] == 'H') return (19);  // HOME
        if (seq[1] == 'F') return (95);  // END
      }
    } else if (seq[0] == 'O') { // ESC O sequences
      if (seq[1] == 'H') return (19); // HOME
      if (seq[1] == 'F') return (95); // END
    }
  }

  return (c);
}

uint8_t cursor(uint8_t onoff) {
  uint8_t old = cursor_onoff;

  cursor_onoff = onoff;

  printf("\e[?25%c", (onoff) ? 'h' : 'l');
  fflush(stdout);

  return (old);
}

uint8_t textcolor(uint8_t color) {
  uint8_t old = color_fg;

  color_fg = color;

  printf("\e[%i;%im", (color == COLOR_WHITE) ? 1 : 0, color + 30);
  fflush(stdout);

  return (old);
}

uint8_t bgcolor(uint8_t color) {
  uint8_t old = color_bg;

  color_bg = color;

  printf("\e[%im", color + 40);
  fflush(stdout);

  return (old);
}

uint8_t bordercolor(uint8_t color) {
  uint8_t old = color_bd;

  color_bd = color;

  return (old);
}

void cclear(uint8_t length) {
  uint8_t i;

  for (i=0; i<length; i++) cputc(' ');
}

void screensize(uint8_t *x, uint8_t *y) {
  *x = screen_w;
  *y = screen_h;
}

int cprintf(const char *format, ...) {
  va_list args;

  va_start(args, format);
  int ret = vcprintf(format, args);
  va_end(args);

  return (ret);
}

int vcprintf(const char *format, va_list ap) {
  char buf[256];

  int ret = vsnprintf(buf, sizeof (buf), format, ap);

  cputs(buf);

  return (ret);
}
