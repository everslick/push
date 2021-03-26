#ifndef _TERM_H_
#define _TERM_H_

#include <stdint.h>

#ifdef VT100
 #include "vt100.h"
#endif

#ifdef ZX
 #include "zxspec.h"
#endif

#ifdef MEGA65
 #include "mega65.h"
#endif

#ifdef HAVE_CONIO
 #include <conio.h>
#endif

#ifndef COLOR_DEFAULT
 #define COLOR_DEFAULT COLOR_GRAY3
#endif

#include "lined.h"

// TERMINAL KEY CODES
#define TERM_KEY_NONE        0
#define TERM_KEY_CTRL_A      1
#define TERM_KEY_CTRL_B      2
#define TERM_KEY_CTRL_C      3
#define TERM_KEY_CTRL_D      4
#define TERM_KEY_CTRL_E      5
#define TERM_KEY_CTRL_F      6
#define TERM_KEY_BELL        7
#define TERM_KEY_BACKSPACE   8
#define TERM_KEY_TAB         9
#define TERM_KEY_LINEFEED   10
#define TERM_KEY_CTRL_K     11
#define TERM_KEY_CTRL_L     12
#define TERM_KEY_ENTER      13
#define TERM_KEY_CTRL_N     14
#define TERM_KEY_CTRL_O     15
#define TERM_KEY_CTRL_P     16
#define TERM_KEY_CTRL_Q     17
#define TERM_KEY_CTRL_R     18
#define TERM_KEY_CTRL_S     19
#define TERM_KEY_CTRL_T     20
#define TERM_KEY_CTRL_U     21
#define TERM_KEY_CTRL_V     22
#define TERM_KEY_CTRL_W     23
#define TERM_KEY_CTRL_X     24
#define TERM_KEY_CTRL_Y     25
#define TERM_KEY_CTRL_Z     26
#define TERM_KEY_ESC        27
#define TERM_KEY_DELETE    127

#define TERM_KEY_LEFT      128
#define TERM_KEY_RIGHT     129
#define TERM_KEY_UP        130
#define TERM_KEY_DOWN      131
#define TERM_KEY_PAGEUP    132
#define TERM_KEY_PAGEDOWN  133
#define TERM_KEY_INSERT    134
#define TERM_KEY_HOME      135
#define TERM_KEY_END       136

#define TERM_KEY_F1        241
#define TERM_KEY_F2        242
#define TERM_KEY_F3        243
#define TERM_KEY_F4        244
#define TERM_KEY_F5        245
#define TERM_KEY_F6        246
#define TERM_KEY_F7        247
#define TERM_KEY_F8        248
#define TERM_KEY_F9        249
#define TERM_KEY_F10       250
#define TERM_KEY_F11       251
#define TERM_KEY_F12       252

uint8_t term_init();
void    term_fini();

uint8_t term_get_key();

void    term_make_beep();
void    term_clear_screen();
void    term_get_screen_size(uint8_t *cols, uint8_t *rows);

void    term_refresh_line(lined_t *l, char *buf, uint8_t len, uint8_t pos);

extern const char *term_hint_cb(lined_t *l);

#endif // _TERM_H_
