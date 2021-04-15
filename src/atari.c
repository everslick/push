#ifdef HAVE_ATARI

#ifndef ATARI
#error "ATARI is only available on XE and XL targets."
#endif

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "term.h"
#include "push.h"

#include "fileio.h"

static char cwd[64];

static char errstr[17];
static int8_t errnum;

int8_t fileio_version(char *buf, uint8_t size) {
  return (-1);
}

char *fileio_getcwd(char *buf, uint8_t size) {
  strncpy(buf, cwd, size);

  return (buf);
}

int8_t fileio_chdir(const char *dir) {
  return (-1);
}

int8_t fileio_mkdir(const char *dir) {
  return (-1);
}

int8_t fileio_rmdir(const char *dir) {
  return (-1);
}

int8_t fileio_mkfs(const char *name) {
  return (-1);
}

int8_t fileio_ls(uint8_t flags, const char *path) {
  return (-1);
}

void fileio_mount(const char *dev, const char *dir) {
}

void fileio_error(const char *cmd) {
  printf(cmd);
  printf(": ");
  printf(errstr);
  printf("\n");
}

#endif // HAVE_ATARI
