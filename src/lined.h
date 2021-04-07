/* lined.h -- VERSION 1.0
 *
 * A linenoise based line editing library for embedded systems.
 *
 * See lined.cpp for more information.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2016-2021, Clemens Kirchgatterer <clemens at 1541 dot org>
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
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
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
 */

#ifndef _LINED_H_
#define _LINED_H_

#define LINED_LENGTH 80

#define LINED_ECHO     (1<<0) /* Update current line while editing. */
#define LINED_HINTS    (1<<1) /* Show hints while editing line. */
#define LINED_HISTORY  (1<<2) /* Enable history browsing while editing. */
#define LINED_COMPLETE (1<<3) /* TAB completion is enabled for editing. */

#ifdef HAVE_COMPLETION
typedef struct completion_t {
  uint8_t len;
  uint8_t index;
  char **cvec;
} completion_t;
#endif

/* The lined_t structure represents the state during line editing.
 * We pass this state to functions implementing specific editing
 * functionalities. */
typedef struct lined_t {
  char    buf[LINED_LENGTH]; /* Edited line buffer. */
  uint8_t pos;               /* Current cursor position in buf. */
  uint8_t len;               /* Current edited line length. */
  uint8_t xpos;              /* Current cursor position on screen. */
  uint8_t cols;              /* Number of columns in terminal. */
  uint8_t rows;              /* Number of rows in terminal. */
  uint8_t flags;             /* ECHO, HINTS, HISTORY, COMPLETE */
  uint8_t plen;              /* Prompt length. */
  uint8_t key;               /* Last pressed key. */
#ifdef HAVE_COMPLETION
  completion_t *lc;          /* Current TAB completion vector. */
#endif
#ifdef HAVE_HISTORY
  int8_t index;              /* The history index we are currently editing. */
#endif
  const char *prompt;        /* Prompt to display. */
} lined_t;

lined_t *lined_init(void);
char    *lined_line(lined_t *l);
void     lined_fini(lined_t *l);

void     lined_edit(lined_t *l, uint8_t key);
void     lined_resize(lined_t *l, uint8_t w, uint8_t h);
void     lined_prompt(lined_t *l, const char *prompt);
void     lined_reset(lined_t *l, uint8_t flags);

void     lined_completion_add(lined_t *l, const char *str);
void     lined_history_add(const char *line);
void     lined_history_len(uint8_t len);

extern void lined_complete_cb(lined_t *l);

#endif // _LINED_H_
