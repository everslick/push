#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#include "lined.h"
#include "term.h"
#include "cli.h"

#include "push.h"

#ifdef HAVE_FILEIO

#ifdef HAVE_SDIEC
#include "sdiec.h"
#endif

#ifdef __CBM__
#include <device.h>
#include <cbm.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif // __CBM__

#endif // HAVE_FILEIO

// busybox
#define LONE_DASH(s)       ((s)[0] == '-' && !(s)[1])
#define NOT_LONE_DASH(s)   ((s)[0] != '-' || (s)[1])
#define LONE_CHAR(s,c)     ((s)[0] == (c) && !(s)[1])
#define NOT_LONE_CHAR(s,c) ((s)[0] != (c) || (s)[1])
#define DOT_OR_DOTDOT(s)   ((s)[0] == '.' && (!(s)[1] || ((s)[1] == '.' && !(s)[2])))

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
  "help\0"   "echo\0"    "parse\0"   "clear\0"
  "reset\0"  "version\0" "pwd\0"     "mount\0" 
  "cd\0"     "ls\0"      "mv\0"      "rm\0" 
  "mkdir\0"  "test\0"    "logout\0"  "exit\0"
  "\0" // end marker

  // TODO
  //"cat\0" "cp\0" "df\0" "dd\0" "mkfs\0"
  //"tail\0" "touch\0" "hd\0" "tetris\0"
};

static const char input[] = {
  "help\r"
  "echo foo    bar     baz\r"
  "parse   this  is a test for   the argc/argv parser\r"
  "version\r"
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

static uint8_t getflags(uint8_t argc, char **argv, const char *optstr) {
  uint8_t i, flags = 0, n = strlen(optstr);
  int opt;

  if (n > 8) n = 8;

  // reset global option index
  optind = 1;

  while ((opt = getopt(argc, argv, optstr)) != -1) {
    for (i=0; i<n; i++) {
      if (opt == optstr[i]) flags |= (1<<i);
    }
  }

  return (flags);
}

/* Find out if the last character of a string matches the one given.
 * Don't underrun the buffer if the string length is 0.
 */
static char *last_char_is(const char *s, uint8_t c) {
  if (s && *s) {
    uint8_t sz = strlen(s) - 1;

    s += sz;
    if ((uint8_t)*s == c) {
      return (char *)s;
    }
  }

  return (NULL);
}

static char *get_last_path_component(const char *path) {
  char *slash = strrchr(path, '/');

  if (!slash || (slash == path && !slash[1])) {
    return (char*)path;
  }

  return (slash + 1);
}

static const char *basename(const char *path) {
  const char *s = strrchr(path, '/');

  if (s) return (s + 1);

  return (path);
}

static char *dirname(char *path) {
  char *slash = last_char_is(path, '/');

  if (slash) {
    while (*slash == '/' && slash != path) {
      *slash-- = '\0';
    }
  }

  return (get_last_path_component(path));
}

static void not_implemented(const char *cmd) {
  printf("%s: not implemented" LF, cmd);
}

static void not_found(const char *cmd) {
  printf("%s: no such file or directory" LF, cmd);
}

static void dir_not_found(const char *cmd) {
  printf("%s: directory not found" LF, cmd);
}

static void missing_arg(const char *cmd) {
  printf("%s: missing argument" LF, cmd);
}

static void cmd_help(uint8_t argc, char **argv) {
  const char *ptr = commands;
  uint8_t n = 1;

  printf("available commands are:" LF " ");

  while (*ptr) {
    printf("%-8s", ptr);
    if ((n++ % 4) == 0) printf(LF " ");
    ptr += strlen(ptr) + 1;
  }
  if ((n % 4) == 0) printf(LF);

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
#ifndef ZXN
#ifndef ZX
  printf("push, version " VERSION LF); return;
#endif // ZX
#endif // ZXN
  not_implemented(*argv);
}

static void cmd_clear(uint8_t argc, char **argv) {
  term_clear_screen();
}

static void cmd_mv(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  if (argc < 3) {
    missing_arg(*argv);
  } else {
    if (rename(argv[1], argv[2])) {
      perror(*argv);
    }
  }
#else
  not_implemented(*argv);
#endif
}

static void cmd_rm(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  uint8_t flags = getflags(argc, argv, "?v");
  char *path;

  if (flags & 0x01) { // ?
    printf("usage: %s [-v] name" LF, *argv);
    return;
  }

  argv += optind;

  if (!*argv) {
    missing_arg("rm");
    return;
  }

  do {
    path = *argv;

    if (flags & 0x02) { // verbose
      printf("rm: removing '%s'" LF, path);
    }

    if (remove(path)) {
      perror("rm");
    }
  } while (*++argv);
#else
  not_implemented(*argv);
#endif
}

static void cmd_mkdir(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  uint8_t flags = getflags(argc, argv, "?v");
  char *path;

  if (flags & 0x01) { // ?
    printf("usage: %s [-v] name" LF, *argv);
    return;
  }

  argv += optind;

  if (!*argv) {
    missing_arg("mkdir");
    return;
  }

  do {
    int mode = 0777;

    path = *argv;

    if (flags & 0x02) { // verbose
      printf("mkdir: creating directory '%s'" LF, path);
    }

#ifdef HAVE_SDIEC
    if (sdiec_mkdir(path)) {
      sdiec_error("mkdir");
    }
#else
    if (mkdir(path, mode)) {
      perror("mkdir");
    }
#endif
  } while (*++argv);
#else
  not_implemented(*argv);
#endif
}

static void cmd_rmdir(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  uint8_t flags = getflags(argc, argv, "?v");
  char *path;

  if (flags & 0x01) { // ?
    printf("usage: %s [-v] name" LF, *argv);
    return;
  }

  argv += optind;

  if (!*argv) {
    missing_arg("rmdir");
    return;
  }

  do {
    path = *argv;

    if (flags & 0x02) { // verbose
      printf("rmdir: removing directory '%s'" LF, path);
    }

#ifdef HAVE_SDIEC
    if (sdiec_rmdir(path)) {
      sdiec_error("rmdir");
    }
#else
    if (rmdir(path) < 0) {
      perror("rmdir");
    }
#endif
  } while (*++argv);
#else
  not_implemented(*argv);
#endif
}

static void cmd_pwd(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  char *pwd;

#ifdef HAVE_SDIEC
  pwd = sdiec_getcwd(scratch, sizeof (scratch));
#else
  pwd = getcwd(scratch, sizeof (scratch));
#endif

  if (pwd) {
    printf("%s" LF, pwd);
  } else {
    perror(*argv);
  }
#else
  not_implemented(*argv);
#endif
}

static void cmd_mount(uint8_t argc, char **argv) {
#ifdef C64
  unsigned char dev = getfirstdevice();

  while (dev != INVALID_DEVICE) {
    printf ("/dev/fd%d on /mnt/%d" LF, dev - 8, dev);
    dev = getnextdevice(dev);
  }
#else
  not_implemented(*argv);
#endif
}

static void cmd_cd(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO

#ifdef __CBM__
 #ifdef HAVE_SDIEC
  if (sdiec_cd(argv[1])) {
    sdiec_error("cd");
  }
 #else // HAVE_SDIEC
  unsigned char dev = getfirstdevice();
  char buf[FILENAME_MAX], *dir = NULL;

  dir = getdevicedir(getfirstdevice(), buf, sizeof (buf));

  if (!dir || chdir(dir)) {
    perror(*argv);
  }
 #endif // HAVE_SDIEC
#else // __CBM__
  if (chdir(argv[1])) {
    perror(*argv);
  }
#endif // __CBM__

#else // HAVE_FILEIO
  not_implemented(*argv);
#endif // HAVE_FILEIO
}

static void cmd_ls(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  uint8_t files = 1;
#ifdef __CBM__
  uint8_t dev = getcurrentdevice();
  struct cbm_dirent entry;
#else
  DIR *dir = opendir(".");
  struct dirent *entry;
#endif

#ifdef __CBM__
  if (cbm_opendir(1, dev)) {
#else
  if (!dir) {
#endif
    perror("ls");
  } else {
#ifdef __CBM__
    if (cbm_readdir(1, &entry)) { // skip parent dir
      cbm_closedir(1);
      return;
    }

    while (!cbm_readdir(1, &entry)) {
      const char *name = entry.name;
#else
    while ((entry = readdir(dir))) {
      const char *name = entry->d_name;
#endif
      uint8_t col = COLOR_DEFAULT;

      if (!strcmp(name, ".") || !strcmp(name, "..")) {
        continue;
      }

#ifdef __CBM__
      if (entry.type == CBM_T_DIR) col = COLOR_BLUE;
#else
      if (entry->d_type == DT_DIR) col = COLOR_BLUE;
#endif

      textcolor(col);
      printf("%-19s", name);
      if ((files++ % 2) == 0) printf(LF);
    }

    if ((files % 2) == 0) printf(LF);

#ifdef __CBM__
    cbm_closedir(1);
#else
    closedir(dir);
#endif
  }
#else
  not_implemented(*argv);
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

  if (!strcmp(c, "cd"))    return ("[<dir>]");
  if (!strcmp(c, "ls"))    return ("[<dir>]");
  if (!strcmp(c, "mv"))    return ("<old> <new>");
  if (!strcmp(c, "rm"))    return ("<name>");
  if (!strcmp(c, "mkdir")) return ("<dir>");
  if (!strcmp(c, "rmdir")) return ("<dir>");
  if (!strcmp(c, "mount")) return ("[<dir>] [<dev>]");
  if (!strcmp(c, "parse")) return ("[<arg1> <arg2> ...]");
  if (!strcmp(c, "echo"))  return ("[<text1> <text2>] ...");

  return (NULL);
}
#endif

uint8_t cli_exec(char *cmd) {
  char *argv[8];
  uint8_t argc;

  argc = parse(cmd, argv, 8);

  if (argc == 0) return (0);

  if (!strcmp(*argv, "exit") || !strcmp(*argv, "logout")) {
    return (1); // exit
  } else if (!strcmp(*argv, "reset")) {
    return (2); // reset
  } else if (!strcmp(*argv, "cd")) {
    cmd_cd(argc, argv);
  } else if (!strcmp(*argv, "ls")) {
    cmd_ls(argc, argv);
  } else if (!strcmp(*argv, "mv")) {
    cmd_mv(argc, argv);
  } else if (!strcmp(*argv, "rm")) {
    cmd_rm(argc, argv);
  } else if (!strcmp(*argv, "mkdir")) {
    cmd_mkdir(argc, argv);
  } else if (!strcmp(*argv, "rmdir")) {
    cmd_rmdir(argc, argv);
  } else if (!strcmp(*argv, "pwd")) {
    cmd_pwd(argc, argv);
  } else if (!strcmp(*argv, "mount")) {
    cmd_mount(argc, argv);
  } else if (!strcmp(*argv, "clear")) {
    cmd_clear(argc, argv);
  } else if (!strcmp(*argv, "echo")) {
    cmd_echo(argc, argv);
  } else if (!strcmp(*argv, "version")) {
    cmd_version(argc, argv);
  } else if (!strcmp(*argv, "help")) {
    cmd_help(argc, argv);
  } else if (!strcmp(*argv, "parse")) {
    cmd_parse(argc, argv);
  } else if (!strcmp(*argv, "test")) {
    term_push_keys(input);
  } else {
#ifdef C64
    exec(*argv, NULL); // will not return if works
#endif

    printf("%s: command not found" LF, *argv);
  }

  return (0);
}
