#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "lined.h"
#include "term.h"
#include "cli.h"

#ifdef HAVE_FILEIO
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef KICKC

static const char commands[] = {
  'h','e','l','p', 0,
  'e','c','h','o', 0,
  'p','a','r','s','e',0,
  'c','l','e','a','r', 0,
  'r','e','s','e','t', 0,
  'v','e','r','s','i','o','n', 0,
  'u','p','t','i','m','e', 0,
  'l','o','g','o','u','t', 0,
  'e','x','i','t', 0,
   0 // end marker
};

#else

static const char commands[] = {
  "help\0"
  "echo\0"
  "parse\0"
  "clear\0"
  "reset\0"
  "version\0"
  "ls\0" 
  "mv\0" 
  "rm\0" 
  "logout\0"
  "exit\0"
  "\0" // end marker

  // TODO
  //"cat\0" "cp\0" "df\0" "dd\0" "ls\0" "mkfs\0"
  //"mv\0" "rm\0" "tail\0" "touch\0" "hd\0" "tetris\0"
};

#endif

#define KEYS                           \
  " c=break b=left  l=cls  k=ceol"  LF \
  " d=exit  f=right o=osd  u=cline" LF \
  " a=home  p=up    r=char w=cword" LF \
  " e=end   n=down  t=swap"         LF

#ifdef KICKC
#define STRT '\''
#else
#define STRT '"'
#endif

static uint8_t parse(char *cmd, char **argv, uint8_t args) {
  uint8_t i, argc = 0, quote = 0, first = 1;
  char *s = cmd, *t = s + strlen(s) - 1;

  while ((t >= s) && (*t && (*t == ' '))) *t-- = 0; // trim end

  for (i=0; i<args; i++) {
    argv[i] = 0;
  }

  while (*s) {
    if (first) {
      first = 0;

      // skip whitespace
      while (*s && (*s == ' ')) s++;

      // skip quotation mark
      if (*s && (*s == STRT)) { s++; quote ^= 1; }

      // start next arg
      *argv++ = s;

      if (++argc == args) {
        return (argc);
      }
    }

    if (*s == STRT) {
      *s++ = 0; // remove and skip
      if (*s == STRT) first = 1;
      quote ^= 1;
    } else if ((!quote) && (*s == ' ')) {
      *s++ = 0; // remove and skip
      first = 1;
    } else {
      s++;
    }
  }

  return (argc);
}

static void not_implemented(const char *cmd) {
  printf("%s: not implemented" LF, cmd);
}

static void not_found(const char *cmd) {
  printf("%s: no such file or directory" LF, cmd);
}

static void missing_arg(const char *cmd) {
  printf("%s: missing argument" LF, cmd);
}

static void open_failed(const char *cmd) {
  printf("%s: open failed" LF, cmd);
}

static void cmd_help(uint8_t argc, char **argv) {
  const char *ptr = commands;

  printf("available commands are:" LF);

  while (*ptr) {
    printf(" %s" LF, ptr);
    ptr += strlen(ptr) + 1;
  }

  printf(LF);
  printf("line editor keys are [ctrl]+[x]:" LF);
  printf(KEYS);
  printf(LF);
}

static void cmd_parse(uint8_t argc, char **argv) {
  uint8_t i;

  for (i=0; i<argc; i++) {
    printf("argv[%i]='%s'" LF, (int)i, argv[i]);
  }
}

static void cmd_echo(uint8_t argc, char **argv) {
  uint8_t i;

  for (i=1; i<argc; i++) {
    printf("%s", argv[i]);
    if (i < argc - 1) printf(" ");
  }
  printf(LF);
}

static void cmd_version(uint8_t argc, char **argv) {
#ifndef ZX
  printf("push, version " VERSION LF);
#else
  not_implemented("version");
#endif
}

static void cmd_clear(uint8_t argc, char **argv) {
  term_clear_screen();
}

static void cmd_rm(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  if (argc < 2) {
    missing_arg("rm");
  } else {
    if (remove(argv[1])) {
      not_found("rm");
    }
  }
#else
  not_implemented("rm");
#endif
}

static void cmd_mv(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  if (argc < 3) {
    missing_arg("rm");
  } else {
    if (rename(argv[1], argv[2])) {
      not_found("mv");
    }
  }
#else
  not_implemented("mv");
#endif
}

static void cmd_ls(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  struct dirent *entry;
  uint8_t files = 0;
  DIR *dir;

  dir = opendir(".");

  if (!dir) {
    open_failed("ls");
  } else {
    while ((entry = readdir(dir))) {
      const char *name = entry->d_name;
      uint8_t col = COLOR_DEFAULT;

      if (!strcmp(name, ".") || !strcmp(name, "..")) {
        continue;
      }

      if (entry->d_type == 2) col = COLOR_BLUE;

      textcolor(col);
      printf("%-15s", name);
      if (files++ % 2) printf(LF);
    }

    if (files % 2) printf(LF);

    closedir(dir);
  }
#else
  not_implemented("ls");
#endif
}

void lined_complete_cb(lined_t *l) {
#ifdef HAVE_COMPLETION
  const char *ptr = commands;
  const char *c = l->buf;
  uint8_t spaces = 0;

  // remove all the leading spaces
  while (c && (*c == ' ')) {
    spaces++;
    c++;
  }

  // TAB completion requires at least one character
  if (strlen(c) < 1) return;

  if (!strncmp(c, "echo", 4)) {
    lined_completion_add(l, "echo foo bar");
  }

  while (*ptr) {
    if (!strncmp(ptr, c, strlen(c))) {
      lined_completion_add(l, ptr);
    }

    ptr += strlen(ptr) + 1;
  }
#endif
}

#ifdef HAVE_HINTS
const char *term_hint_cb(lined_t *l) {
  const char *c = l->buf;

  // remove all the leading spaces
  while (c && (*c == ' ')) c++;

  if (!strcmp(c, "parse")) return ("[<arg1> <arg2> ...]");
  if (!strcmp(c, "echo"))  return ("<text>");

  return (NULL);
}
#endif

uint8_t cli_exec(char *cmd) {
  char *argv[8];
  uint8_t argc;

  argc = parse(cmd, argv, 8);

  if (argc == 0) return (0);

  if (!strcmp(argv[0], "exit") || !strcmp(argv[0], "logout")) {
    return (1); // exit
  } else if (!strcmp(argv[0], "reset")) {
    return (2); // reset
  } else if (!strcmp(argv[0], "help")) {
    cmd_help(argc, argv);
  } else if (!strcmp(argv[0], "echo")) {
    cmd_echo(argc, argv);
  } else if (!strcmp(argv[0], "parse")) {
    cmd_parse(argc, argv);
  } else if (!strcmp(argv[0], "clear")) {
    cmd_clear(argc, argv);
  } else if (!strcmp(argv[0], "version")) {
    cmd_version(argc, argv);
  } else if (!strcmp(argv[0], "ls")) {
    cmd_ls(argc, argv);
  } else if (!strcmp(argv[0], "mv")) {
    cmd_mv(argc, argv);
  } else if (!strcmp(argv[0], "rm")) {
    cmd_rm(argc, argv);
  } else {
    printf("%s: command not found" LF, argv[0]);
  }

  return (0);
}
