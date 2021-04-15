#ifdef HAVE_ESXDOS

#ifndef ZXN
#error "ESXDOS is only available on ZXNext."
#endif

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <arch/zxn/esxdos.h>

#include <sys/stat.h>
#include <dirent.h>

#include "term.h"
#include "push.h"

static char errstr[17];
static int8_t errnum;

int rename(const char *old, const char *new) {
  return (-1);
}

int8_t fileio_version(char *buf, uint8_t size) {
  return (-1);
}

char *fileio_getcwd(char *buf, uint8_t size) {
  char cwd[ESXDOS_PATH_MAX + 1];

  if (buf && size) {
    buf[0] = '\0';
  }

  if (esx_f_getcwd(cwd) != 0xff) {
    strncpy(buf, cwd, size);
  }

  return (buf);
}

int8_t fileio_chdir(const char *dir) {
  return (esx_f_chdir(dir));
}

int8_t fileio_mkdir(const char *dir) {
  return (esx_f_mkdir((char *)dir));
}

int8_t fileio_rmdir(const char *dir) {
  return (esx_f_unlink(dir));
}

int8_t fileio_mkfs(const char *name) {
  return (-1);
}

int8_t fileio_ls(uint8_t flags, char *path) {
  uint8_t col, listlong = 0, listall = 0, columns = 2;
  char *time = "2000/12/31 00:00";
  struct esx_dirent entry;
  char *name, type;
  uint8_t files = 1;
  size_t size;

  uint8_t dir = esx_f_opendir(path);

  //  1  2  4  8
  // -? -a -l -1

  if (flags & 0x02) { listall = 1;               }
  if (flags & 0x04) { columns = 1; listlong = 1; }
  if (flags & 0x08) { columns = 1;               }

  if (dir == 0xff) {
    uint8_t f = esx_f_open(name, 'r');

    if (f != 0xff) {
      struct esx_stat st;

      if (esx_f_fstat(f, &st) < 0) {
        perror("ls");
      } else {
        if (listlong) {
          printf("F %6u %s %s\n", st.size, time, name);
        } else {
          printf("%-19s\n", name);
        }
      }

      esx_f_close(f);
    }
  } else { 
    while (!esx_f_readdir(dir, &entry)) {
      struct esx_dirent_slice *info; // info -> attr, date, size
      col = COLOR_DEFAULT;
      type = 'F';

      if ((entry.name[0] == '.') && (!listall)) continue;

      info = (struct esx_dirent_slice *)&entry.name[ESXDOS_NAME_MAX+1];

      if (entry.attr & ESXDOS_ATTR_DIR) {
        type = 'D';
        col = COLOR_BLUE;
      }

      size = info->size;

      if (listlong) {
        textcolor(COLOR_DEFAULT); printf("%c %6u %s ", type, info->size, time);
        textcolor(col);           printf("%s", entry.name);
      } else {
        textcolor(col);           printf("%-19s", entry.name);
      }

      if ((files++ % columns) == 0) printf("\n");
    }

    esx_f_close(dir);
  }

  if ((columns > 1) && (files % columns) == 0) printf("\n");

  return (0);
}

void fileio_mount(const char *dev, const char *dir) {
}

void fileio_error(const char *cmd) {
  printf(cmd);
  printf(": ");
  printf(errstr);
  printf("\n");
}

#endif // HAVE_ESXDOS
