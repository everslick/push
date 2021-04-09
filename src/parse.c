#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "push.h"

uint8_t parse_opterr = 1,  /* if error message should be printed */
        parse_optind = 1,  /* index into parent argv vector */
        parse_optopt,      /* character checked for validity */
        parse_optreset;    /* reset getopt */

char *parse_optarg;        /* argument associated with option */

#define  BADCH  '?'
#define  BADARG ':'
#define  EMSG   ""

int8_t parse_getopt(uint8_t nargc, char **nargv, const char *ostr) {
  static char *place = EMSG;    /* option letter processing */
  char *oli;                    /* option letter list index */

  if (parse_optreset || !*place) {    /* update scanning pointer */
    parse_optreset = 0;
    if (parse_optind >= nargc || *(place = nargv[parse_optind]) != '-') {
      place = EMSG;
      return (-1);
    }

    if (place[1] && *++place == '-') {  /* found "--" */
      ++parse_optind;
      place = EMSG;
      return (-1);
    }
  }                             /* option letter okay? */

  if ((parse_optopt = *place++) == ':' ||
      !(oli = strchr(ostr, parse_optopt))) {
    /*
     * if the user didn't specify '-' as an option,
     * assume it means -1.
     */
    if (parse_optopt == '-') return (-1);
    if (!*place) ++parse_optind;
    if (parse_opterr && *ostr != ':') {
      printf("illegal option -- %c\n",  parse_optopt);
    }

    return (BADCH);
  }
  if (*++oli != ':') {          /* don't need argument */
    parse_optarg = NULL;
    if (!*place) ++parse_optind;
  } else {                      /* need an argument */
    if (*place) {               /* no white space */
      parse_optarg = place;
    } else if (nargc <= ++parse_optind) {  /* no arg */
      place = EMSG;
      if (*ostr == ':') return (BADARG);
      if (parse_opterr) {
        printf("option requires an argument -- %c\n", parse_optopt);
      }

      return (BADCH);
    } else {                    /* white space */
      parse_optarg = nargv[parse_optind];
    }
    place = EMSG;
    ++parse_optind;
  }

  return (parse_optopt);              /* dump back option letter */
}

uint8_t parse_command(char *cmd, char **argv, uint8_t args) {
  uint8_t i, argc = 0, quote = 0, first = 1;
  char *s = cmd, *t = s + strlen(s) - 1;

  while ((t >= s) && (*t && (*t == ' '))) *t-- = '\0'; /* trim end */

  for (i=0; i<args; i++) {
    argv[i] = '\0';
  }

  while (*s) {
    if (first) {
      first = 0;

      /* skip whitespace */
      while (*s && (*s == ' ')) s++;

      /* skip quotation mark */
      if (*s && (*s == '"')) { s++; quote ^= 1; }

      /* start next arg */
      *argv++ = s;

      if (++argc == args) {
        return (argc);
      }
    }

    if (*s == '"') {
      *s++ = '\0'; /* remove and skip */
      if (*s == '"') first = 1;
      quote ^= 1;
    } else if ((!quote) && (*s == ' ')) {
      *s++ = '\0'; /* remove and skip */
      first = 1;
    } else {
      s++;
    }
  }

  return (argc);
}

uint8_t parse_optflags(uint8_t argc, char **argv, const char *optstr) {
  uint8_t n, i, flags = 0;
  int opt;

  n = strlen(optstr);
  if (n > 8) n = 8;

  /* reset global option index */
  parse_optreset = 1;

  while ((opt = parse_getopt(argc, argv, optstr)) != -1) {
    for (i=0; i<n; i++) {
      if (opt == optstr[i]) flags |= (1<<i);
    }
  }

  return (flags);
}

const char *parse_dirname(const char *path) {
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

const char *parse_basename(const char *path) {
  const char *slash = strrchr(path, '/');

  if (slash) return (slash + 1);

  return (path);
}

char *parse_realpath(const char *path, char *unused) {
  uint8_t i, len, rel, ind = 0, sz = 0;
  char buf[64], *tokv[8], *ptr = NULL;

  strcpy(buf, path);
  rel = (*path == '/') ? 0 : 1;

  ptr = strtok(buf, "/");
  while (ptr != NULL) {
    if (strcmp(ptr, "..") == 0) {
      if (ind > 0) {
        ind--;
      }
    } else if (strcmp(ptr, ".") != 0) {
      tokv[ind++] = ptr;

      if (ind >= 8) return (NULL);
    }
    ptr = strtok(NULL, "/");
  }

  ptr = scratch;
  for (i=0; i<ind; i++) {
    len = strlen(tokv[i]);

    if (i > 0 || !rel) {
      if (++sz >= sizeof (scratch)) return (NULL);
      *ptr++ = '/';
    }

    sz += len;
    if (sz >= sizeof (scratch)) return (NULL);

    strcpy(ptr, tokv[i]);
    ptr += len;
  }

  if (ptr == scratch) {
    if (++sz >= sizeof (scratch)) return (NULL);
    *ptr++ = rel ? '.' : '/';
  }
  *ptr = '\0';

  return (scratch);
}

