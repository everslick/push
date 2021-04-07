#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
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

static const char commands[] = {
  "help\0"     "echo\0"     "sleep\0"    "clear\0"
  "reset\0"    "version\0"  "pwd\0"      "mount\0" 
  "cd\0"       "ls\0"       "mv\0"       "rm\0" 
  "realpath\0" "basename\0" "dirname\0"  "mkdir\0"
  "parse\0"    "test\0"     "logout\0"   "exit\0"
  "\0" // end marker
};

static const char input[] = {
  "help\r"
  "sleep 2\r"
  "clear\r"
  "echo foo    bar     baz\r"
  "parse   this  is a    test for the    \" very fine \"  argc/argv  parser\r"
  "dirname /foo/bar/baz.txt\r"
  "basename /foo/bar/baz.txt\r"
  "realpath ./foo/../bar/../foobar/baz.txt\r"
  "ls\r"
  "mkdir foo\r"
  "cd foo\r"
  "ls\r"
  "mkdir bar\r"
  "ls\r"
  "rmdir bar\r"
  "ls\r"
  "cd ..\r"
  "rmdir foo\r"
  "version\r"
  "echo bye!\r"
  "sleep 5\r"
  "reset\r"
};

#define KEYS                           \
  " c=break b=left  l=cls  k=ceol"  LF \
  " d=exit  f=right o=osd  u=cline" LF \
  " a=home  p=up    r=char w=cword" LF \
  " e=end   n=down  t=swap"         LF

static uint8_t parse(char *cmd, char **argv, uint8_t args) {
  uint8_t i, argc = 0, quote = 0, first = 1;
  char *s = cmd, *t = s + strlen(s) - 1;

  while ((t >= s) && (*t && (*t == ' '))) *t-- = '\0'; // trim end

  for (i=0; i<args; i++) {
    argv[i] = '\0';
  }

  while (*s) {
    if (first) {
      first = 0;

      // skip whitespace
      while (*s && (*s == ' ')) s++;

      // skip quotation mark
      if (*s && (*s == '"')) { s++; quote ^= 1; }

      // start next arg
      *argv++ = s;

      if (++argc == args) {
        return (argc);
      }
    }

    if (*s == '"') {
      *s++ = '\0'; // remove and skip
      if (*s == '"') first = 1;
      quote ^= 1;
    } else if ((!quote) && (*s == ' ')) {
      *s++ = '\0'; // remove and skip
      first = 1;
    } else {
      s++;
    }
  }

  return (argc);
}

static uint8_t getflags(uint8_t argc, char **argv, const char *optstr) {
  uint8_t n, i, flags = 0;
  int opt;

  n = strlen(optstr);
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

static const char *dirname(const char *path) {
  char *s, *slash = NULL, *ptr = scratch;

  strcpy(ptr, path);

  s = ptr + strlen(ptr) - 1;
  if (*s == '/') slash = s;;

  if (slash) {
    while ((*slash == '/') && (slash != ptr)) {
      *slash-- = '\0';
    }
  }

  slash = strrchr(ptr, '/');

  if (slash && (slash != ptr)) {
    *slash = '\0';
  } else {
    if (*ptr != '/') *ptr = '.';
    ptr[1] = '\0';
  }

  return (ptr);
}

static const char *basename(const char *path) {
  const char *slash = strrchr(path, '/');

  if (slash) return (slash + 1);

  return (path);
}

static char *realpath(const char *path, char *unused) {
  uint8_t l, i, rel, sz = 0, ti = 0;
  char buf[32], *tokv[8], *ptr;

  strcpy(buf, path);
  rel = (*path == '/') ? 0 : 1;

  ptr = strtok(buf, "/");
  while (ptr != NULL) {
    if (strcmp(ptr, "..") == 0) {
      if (ti > 0) {
        ti--;
      }
    } else if (strcmp(ptr, ".") != 0) {
      tokv[ti++] = ptr;

      if (ti >= 8) return (NULL);
    }
    ptr = strtok(NULL, "/");
  }

  ptr = scratch;
  for (i=0; i<ti; i++) {
    l = strlen(tokv[i]);

    if (i > 0 || !rel) {
      if (++sz >= sizeof (scratch)) return (NULL);
      *ptr++ = '/';
    }

    sz += l;
    if (sz >= sizeof (scratch)) return (NULL);

    strcpy(ptr, tokv[i]);
    ptr += l;
  }

  if (ptr == scratch) {
    if (++sz >= sizeof (scratch)) return (NULL);
    *ptr++ = rel ? '.' : '/';
  }
  *ptr = '\0';

  return (scratch);
}

static void not_implemented(const char *cmd) {
  printf("%s: not implemented" LF, cmd);
}

static void missing_arg(const char *cmd) {
  printf("%s: missing argument" LF, cmd);
}

static void cmd_help(uint8_t argc, char **argv) {
  const char *ptr = commands;
  uint8_t n = 1;

  printf("available commands:" LF " ");

  while (*ptr) {
    printf("%-10s", ptr);
    if ((n++ % 3) == 0) printf(LF " ");
    ptr += strlen(ptr) + 1;
  }
  if ((n % 3) == 0) printf(LF);

  printf(LF);
  printf("line editor keys ([ctrl]+[x]):" LF);
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

static void cmd_sleep(uint8_t argc, char **argv) {
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

  sleep(atoi(argv[1]));
}

static void cmd_mv(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  if (argc < 3) {
    missing_arg(*argv);
    return;
  }

  if (rename(argv[1], argv[2])) {
    perror(*argv);
  }
#else
  not_implemented(*argv);
#endif
}

static void cmd_rm(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  uint8_t flags;
  char *path;

  flags = getflags(argc, argv, "?v");

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
  uint8_t flags;
  char *path;

  flags = getflags(argc, argv, "?v");

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
  uint8_t flags;
  char *path;

  flags = getflags(argc, argv, "?v");

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

static void cmd_realpath(uint8_t argc, char **argv) {
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

  printf("%s" LF, realpath(argv[1], NULL));
}

static void cmd_basename(uint8_t argc, char **argv) {
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

  printf("%s" LF, basename(argv[1]));
}

static void cmd_dirname(uint8_t argc, char **argv) {
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

  printf("%s" LF, dirname(argv[1]));
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
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

#ifdef HAVE_SDIEC
  if (sdiec_chdir(argv[1])) {
    sdiec_error(*argv);
  }
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

  if (!strcmp(c, "cd"))       return ("<path>");
  if (!strcmp(c, "mv"))       return ("<old> <new>");
  if (!strcmp(c, "rm"))       return ("<name>");
  if (!strcmp(c, "mkdir"))    return ("<dir>");
  if (!strcmp(c, "rmdir"))    return ("<dir>");
  if (!strcmp(c, "realpath")) return ("<path>");
  if (!strcmp(c, "basename")) return ("<path>");
  if (!strcmp(c, "dirname"))  return ("<path>");
  if (!strcmp(c, "mount"))    return ("[<dir>] [<dev>]");
  if (!strcmp(c, "parse"))    return ("[<arg1> <arg2> ...]");
  if (!strcmp(c, "echo"))     return ("[<text1> <text2>] ...");
  if (!strcmp(c, "sleep"))    return ("<sec>");

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
  } else if (!strcmp(*argv, "realpath")) {
    cmd_realpath(argc, argv);
  } else if (!strcmp(*argv, "basename")) {
    cmd_basename(argc, argv);
  } else if (!strcmp(*argv, "dirname")) {
    cmd_dirname(argc, argv);
  } else if (!strcmp(*argv, "mount")) {
    cmd_mount(argc, argv);
  } else if (!strcmp(*argv, "clear")) {
    cmd_clear(argc, argv);
  } else if (!strcmp(*argv, "sleep")) {
    cmd_sleep(argc, argv);
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
