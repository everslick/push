#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "parse.h"
#include "lined.h"
#include "term.h"
#include "cli.h"

#include "push.h"

#ifdef HAVE_FILEIO

#if defined(HAVE_SDIEC) || defined(HAVE_DIVMMC)
#include "fileio.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#endif // HAVE_FILEIO

#define _mkstr_(_s_)  #_s_                                                      
#define mkstr(_s_)    _mkstr_(_s_)

static const char commands[] = {
  "help\0"     "echo\0"     "sleep\0"    "clear\0"
  "reset\0"    "version\0"  "pwd\0"      "mount\0" 
  "cd\0"       "ls\0"       "mv\0"       "rm\0" 
  "realpath\0" "basename\0" "dirname\0"  "mkdir\0"
  "rmdir\0"    "parse\0"    "test\0"     "logout\0"
  "exit\0"
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

#ifdef ZX

#define KEYS                 \
  " q=break y=left  s=del\n" \
  " d=exit  i=right w=osd\n" \
  " a=home  u=up\n"          \
  " e=end   g=down\n"      

#else

#define KEYS                          \
  " c=break b=left  l=cls  k=ceol\n"  \
  " d=exit  f=right o=osd  u=cline\n" \
  " a=home  p=up    r=char w=cword\n" \
  " e=end   n=down  t=swap\n"

#endif

static void not_implemented(const char *cmd) {
  printf("%s: not implemented\n", cmd);
}

static void missing_arg(const char *cmd) {
  printf("%s: missing argument\n", cmd);
}

static void cmd_help(uint8_t argc, char **argv) {
  const char *ptr = commands;
  uint8_t n = 1;

  printf("available commands:\n" " ");

  while (*ptr) {
    printf("%-10s", ptr);
    if ((n++ % 3) == 0) printf("\n ");
    ptr += strlen(ptr) + 1;
  }
  if ((n % 3) == 0) printf("\n");

  printf("\n");
  printf("line editor keys ([ctrl]+[x]):\n");
  printf(KEYS);
  printf("\n");
}

static void cmd_parse(uint8_t argc, char **argv) {
  uint8_t i;

  for (i=0; i<argc; i++) {
    printf("argv[%u]='%s'\n", i, argv[i]);
  }
}

static void cmd_echo(uint8_t argc, char **argv) {
  uint8_t i;

  for (i=1; i<argc; i++) {
    printf("%s", argv[i]);
    if (i < argc - 1) printf(" ");
  }
  printf("\n");
}

static void cmd_version(uint8_t argc, char **argv) {
  printf("push, version "
    mkstr(VERSION)   " ("
    mkstr(MACHINE)   "-"
    mkstr(TOOLCHAIN) ")\n"
  );
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

  flags = parse_optflags(argc, argv, "?v");

  if (flags & 0x01) { // ?
    printf("usage: %s [-v] name\n", *argv);
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
      printf("rm: removing '%s'\n", path);
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

  flags = parse_optflags(argc, argv, "?v");

  if (flags & 0x01) { // ?
    printf("usage: %s [-v] name\n", *argv);
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
      printf("mkdir: creating directory '%s'\n", path);
    }

#if defined(HAVE_SDIEC) || defined(HAVE_DIVMMC)
    if (fileio_mkdir(path)) {
      fileio_error("mkdir");
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

  flags = parse_optflags(argc, argv, "?v");

  if (flags & 0x01) { // ?
    printf("usage: %s [-v] name\n", *argv);
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
      printf("rmdir: removing directory '%s'\n", path);
    }

#if defined(HAVE_SDIEC) || defined(HAVE_DIVMMC)
    if (fileio_rmdir(path)) {
      fileio_error("rmdir");
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

#if defined(HAVE_SDIEC) || defined(HAVE_DIVMMC)
  pwd = fileio_getcwd(scratch, sizeof (scratch));
#else
  pwd = getcwd(scratch, sizeof (scratch));
#endif

  if (pwd) {
    printf("%s\n", pwd);
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

  printf("%s\n", parse_realpath(argv[1]));
}

static void cmd_basename(uint8_t argc, char **argv) {
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

  printf("%s\n", parse_basename(argv[1]));
}

static void cmd_dirname(uint8_t argc, char **argv) {
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

  printf("%s\n", parse_dirname(argv[1]));
}

static void cmd_cd(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  if (argc < 2) {
    missing_arg(*argv);
    return;
  }

#if defined(HAVE_SDIEC) || defined(HAVE_DIVMMC)
  if (fileio_chdir(argv[1])) {
    fileio_error(*argv);
  }
#else
  if (chdir(argv[1])) {
    perror(*argv);
  }
#endif

#else // HAVE_FILEIO
  not_implemented(*argv);
#endif // HAVE_FILEIO
}

static void cmd_ls(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO
  uint8_t header, flags, listlong = 0, listall = 0, columns = 4;

  flags = parse_optflags(argc, argv, "?al1");

  if (flags & 0x01) { // ?
    printf("usage: %s [-a] [-l] [-1] [path]\n", *argv);
    return;
  }

  if (flags & 0x02) { listall = 1;               }
  if (flags & 0x04) { columns = 1; listlong = 1; }
  if (flags & 0x08) { columns = 1;               }

  argv += optind;

  header = (argv[0] && argv[1]);

  do {
    char *path = *argv;

    if (!path) path = ".";
    if (header) printf("%s:\n", path);

#if defined(HAVE_SDIEC) || defined(HAVE_DIVMMC)
    fileio_ls(flags, path);
#else
    DIR *dir = opendir(path);
    struct dirent *entry;
    uint8_t files = 1;

    if (!dir) {
      perror("ls");
      return;
    }

    while ((entry = readdir(dir))) {
      const char *time = "2000/12/31 00:00";
      uint8_t col = COLOR_DEFAULT;
      const char *type = "FILE";
      size_t size = 0;
      struct stat st;

      if ((entry->d_name[0] == '.') && (!listall)) continue;

      if (entry->d_type == DT_DIR) {
        type = "DIR ";
        col = COLOR_BLUE;
      } else {
        stat(entry->d_name, &st);
        size = st.st_size;
      }

      if (listlong) {
        textcolor(COLOR_DEFAULT); printf("%s %6u %s ", type, size, time);
        textcolor(col);           printf("%s", entry->d_name);
      } else {
        textcolor(col);           printf("%-19s", entry->d_name);
      }

      if ((files++ % columns) == 0) printf("\n");
    }

    if ((columns > 1) && (files % columns) == 0) printf("\n");

    closedir(dir);
#endif
    if (header && argv[1]) printf("\n");
  } while (*++argv);

#else // HAVE_FILEIO
  not_implemented(*argv);
#endif // HAVE_FILEIO
}

static void cmd_mount(uint8_t argc, char **argv) {
#ifdef HAVE_FILEIO

#if defined(HAVE_SDIEC) || defined(HAVE_DIVMMC)
  fileio_mount(NULL, NULL);
#endif

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

  argc = parse_command(cmd, argv, 8);

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
#ifdef __CBM__
    if (argv[0][0] == '$') {
      argc = parse_command("ls -la $", argv, 8);
      cmd_ls(argc, argv);

      return (0);
    }

    exec(*argv, NULL); // will not return
#endif

    printf("%s: command not found\n", *argv);
  }

  return (0);
}
