#ifdef __CBM__

#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include <cbm.h>

#include "sdiec.h"
#include "term.h"
#include "push.h"

static char cwd[64];

static char errstr[17];
static int8_t errnum;

#include <stdlib.h>
#include <string.h>

static int8_t snd_drv_cmd(const char *format, ...) {
  uint8_t s = 3, d = 0;
  va_list args;
  int8_t n;

  va_start(args, format);
  vsnprintf(scratch, sizeof (scratch), format, args);
  va_end(args);

  n = cbm_open(15, 8, 15, scratch);
  printf("sending: '%s'" LF, scratch);

  if (n < 0) {
    errnum = n;
    strcpy(errstr, "open failed");
    return (-1); // open failed
  }

  n = cbm_read(15, scratch, (sizeof (scratch)) - 1);
  scratch[n < 0 ? 0 : n] = '\0';

  if (n < 0) {
	  cbm_close(15);
    errnum = n;
    strcpy(errstr, "read failed");
    return (-2); // read failed
  }

  printf("reading: %s", scratch);

  errnum = ((scratch[0] - '0') * 10) + (scratch[1] - '0');

  n -= 7;                     // cut ",00,00\0"
  if (scratch[s] == ' ') s++; // skip possible ' '
  while (s < n) errstr[d++] = scratch[s++];
  errstr[d] = '\0';

  printf("status: '%s' (%u)" LF, errstr, errnum);

	cbm_close(15);

  return (errnum);
}

int8_t sdiec_version(char *buf, uint8_t size) {
  int8_t err = snd_drv_cmd("x?");

  if (!err) {
    strncpy(buf, scratch, size);
  }

  return (err);
}

char *sdiec_getcwd(char *buf, uint8_t size) {
  strncpy(buf, cwd, size);

  return (buf);
}

int8_t sdiec_chdir(const char *dir) {
  if (!strcmp(dir, "..")) {
    return (snd_drv_cmd("cd:\x5f"));
  }

  return (snd_drv_cmd("cd:%s", dir));
}

int8_t sdiec_mkdir(const char *dir) {
  return (snd_drv_cmd("md:%s", dir));
}

int8_t sdiec_rmdir(const char *dir) {
  return (snd_drv_cmd("rd:%s", dir));
}

int8_t sdiec_mkfs(const char *name) {
  return (snd_drv_cmd("n:%s,01", name));
}

void sdiec_error(const char *cmd) {
  printf(cmd);
  printf(": ");
  printf(errstr);
  printf(LF);
}

#endif
