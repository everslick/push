#ifdef HAVE_SDIEC

#ifndef __CBM__
#error "SDIEC is only available on CBM targets."
#endif

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <device.h>
#include <cbm.h>

#include "term.h"
#include "push.h"

#include "fileio.h"

static char cwd[64];

static char errstr[17];
static int8_t errnum;

//#define VERBOSE

static int8_t snd_drv_cmd(const char *format, ...) {
  uint8_t s = 3, d = 0;
  va_list args;
  int8_t n;

  va_start(args, format);
  vsnprintf(scratch, sizeof (scratch), format, args);
  va_end(args);

  n = cbm_open(15, 8, 15, scratch);
#ifdef VERBOSE
  printf("sending: '%s'\n", scratch);
#endif

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

#ifdef VERBOSE
  printf("reading: %s", scratch);
#endif

  errnum = ((scratch[0] - '0') * 10) + (scratch[1] - '0');

  n -= 7;                     // cut ",00,00\0"
  if (scratch[s] == ' ') s++; // skip possible ' '
  while (s < n) errstr[d++] = scratch[s++];
  errstr[d] = '\0';

#ifdef VERBOSE
  printf("status: '%s' (%u)\n", errstr, errnum);
#endif

	cbm_close(15);

  return (errnum);
}

int8_t fileio_version(char *buf, uint8_t size) {
  int8_t err = snd_drv_cmd("x?");

  if (!err) {
    strncpy(buf, scratch, size);
  }

  return (err);
}

char *fileio_getcwd(char *buf, uint8_t size) {
  strncpy(buf, cwd, size);

  return (buf);
}

int8_t fileio_chdir(const char *dir) {
  if (!strcmp(dir, "..")) {
    return (snd_drv_cmd("cd:\x5f"));
  }

  return (snd_drv_cmd("cd:%s", dir));
}

int8_t fileio_mkdir(const char *dir) {
  return (snd_drv_cmd("md:%s", dir));
}

int8_t fileio_rmdir(const char *dir) {
  return (snd_drv_cmd("rd:%s", dir));
}

int8_t fileio_mkfs(const char *name) {
  return (snd_drv_cmd("n:%s,01", name));
}

int8_t fileio_ls(uint8_t flags, const char *path) {
  uint8_t col, listlong = 0, listall = 0, columns = 2;
  uint8_t files = 1, dev = getcurrentdevice();
  struct cbm_dirent entry;
  char *name, type;
  size_t size;

  if (path[0] == '.' && !path[1]) {
    path = "$";
  }

  if (cbm_opendir(1, dev, path)) {
    perror("ls");
    return (-1);
  }

  //  1  2  4  8
  // -? -a -l -1

  if (flags & 0x02) { listall = 1;               }
  if (flags & 0x04) { columns = 1; listlong = 1; }
  if (flags & 0x08) { columns = 1;               }

  while (!cbm_readdir(1, &entry)) {
    char *time = "2000/12/31 00:00";
    col = COLOR_DEFAULT;
    type = 'F';

    if ((entry.name[0] == '.') && (!listall)) continue;

    if (entry.type == CBM_T_DIR) {
      type = 'D';
      col = COLOR_BLUE;
    } else if (entry.type == CBM_T_HEADER) {
      if (!listall) continue;

      type = 'H';
      col = COLOR_CYAN;
      revers(1);
    }

    if (listlong && (type != 'H')) {
      textcolor(COLOR_DEFAULT);
      printf("%c %6u %s ", type, entry.size, time);
      textcolor(col);
      printf("%s", entry.name);
    } else {
      textcolor(col);
      printf("%-19s", entry.name);
    }

    if (type == 'H') {
      revers(0);
      printf("\n");
    } else {
      if ((files++ % columns) == 0) {
        printf("\n");
      }
    }
  }

  if ((columns > 1) && (files % columns) == 0) {
    printf("\n");
  }

  cbm_closedir(1);

  return (0);
}

void fileio_mount(const char *dev, const char *dir) {
  uint8_t device = getfirstdevice();

  while (device != INVALID_DEVICE) {
    printf ("/dev/fd%u on /mnt/%u\n", device - 8, device);
    device = getnextdevice(device);
  }
}

void fileio_error(const char *cmd) {
  printf(cmd);
  printf(": ");
  printf(errstr);
  printf("\n");
}

#endif // HAVE_SDIEC
