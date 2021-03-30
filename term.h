#ifndef _TERM_H_
#define _TERM_H_

#include <stdint.h>

#ifdef LINUX
 #include "linux.h"
#endif

#ifdef C64
#endif

#ifdef ZX
 #include "zxspec.h"
#endif

#ifdef M65
 #include "mega65.h"
#endif

#ifdef HAVE_CONIO
 #include <conio.h>
#endif

#ifndef COLOR_DEFAULT
 #define COLOR_DEFAULT COLOR_GRAY3
#endif

#ifndef LF
 #define LF "\r\n"
#endif

#include "lined.h"

// TERMINAL KEY CODES
#define TERM_KEY_NONE        0
#define TERM_KEY_CTRL_A      1 // HOME
#define TERM_KEY_CTRL_B      2 // LEFT
#define TERM_KEY_CTRL_C      3 // BREAK
#define TERM_KEY_CTRL_D      4 // LOGOUT
#define TERM_KEY_CTRL_E      5 // END
#define TERM_KEY_CTRL_F      6 // RIGHT
#define TERM_KEY_BELL        7
#define TERM_KEY_BACKSPACE   8
#define TERM_KEY_TAB         9
#define TERM_KEY_LINEFEED   10
#define TERM_KEY_CTRL_K     11 // DEL TO EOL
#define TERM_KEY_CTRL_L     12 // CLS
#define TERM_KEY_ENTER      13
#define TERM_KEY_CTRL_N     14 // DOWN
#define TERM_KEY_CTRL_O     15 // TOGGLE OSD
#define TERM_KEY_CTRL_P     16 // UP
#define TERM_KEY_CTRL_Q     17
#define TERM_KEY_CTRL_R     18 // SWITCH CHARSET
#define TERM_KEY_CTRL_S     19
#define TERM_KEY_CTRL_T     20 // SWAP LAST CHARS
#define TERM_KEY_CTRL_U     21 // DEL WHOLE LINE
#define TERM_KEY_CTRL_V     22
#define TERM_KEY_CTRL_W     23 // DEL LAST WORD
#define TERM_KEY_CTRL_X     24
#define TERM_KEY_CTRL_Y     25
#define TERM_KEY_CTRL_Z     26
#define TERM_KEY_ESC        27
#define TERM_KEY_DELETE    127

#define TERM_KEY_F1        241
#define TERM_KEY_F2        242
#define TERM_KEY_F3        243
#define TERM_KEY_F4        244
#define TERM_KEY_F5        245
#define TERM_KEY_F6        246
#define TERM_KEY_F7        247
#define TERM_KEY_F8        248

void    term_init();
void    term_fini();

void    term_make_beep();
void    term_clear_screen();
void    term_screen_size(uint8_t *cols, uint8_t *rows);

uint8_t term_get_key(lined_t *l);
void    term_refresh_line(lined_t *l, char *buf, uint8_t len, uint8_t pos);

extern const char *term_hint_cb(lined_t *l);

#endif // _TERM_H_
