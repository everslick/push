#ifdef HAVE_DIVMMC

#ifndef Z88DK
#error "DIVMMC is only available on Z80 targets."
#endif

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <arch/zx/esxdos.h>

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
  uint8_t drv = esxdos_m_getdrv();
  struct esxdos_device ed;
  int8_t err = 0;

  err = esxdos_disk_info(drv, &ed);

  if (!err) {
    snprintf(buf, size, "%u.%u", ed.path>>3, ed.path & 0x03);
  } else {
    snprintf(buf, size, "error: %i", err);
  }

  return (err);
}

char *fileio_getcwd(char *buf, uint8_t size) {
  if (buf && size) {
    buf[0] = '\0';
  }

  if (size >= ESXDOS_PATH_MAX) {
    esxdos_f_getcwd(buf);
  } else {
    char cwd[ESXDOS_PATH_MAX + 1];

    if (esxdos_f_getcwd(cwd) != 0xff) {
      strncpy(buf, cwd, size);
    }
  }

  return (buf);
}

int8_t fileio_chdir(const char *dir) {
  return (esxdos_f_chdir(dir));
}

int8_t fileio_mkdir(const char *dir) {
  //return (esxdos_f_mkdir((char *)dir));
  return (-1);
}

int8_t fileio_rmdir(const char *dir) {
  return (esxdos_f_unlink(dir));
}

int8_t fileio_mkfs(const char *name) {
  return (-1);
}

int8_t fileio_ls(uint8_t flags, char *path) {
  uint8_t col, listlong = 0, listall = 0, columns = 2;
  char *time = "2000/12/31 00:00";
  struct esxdos_dirent entry;
  char *name, type;
  uint8_t files = 1;
  size_t size;

  uint8_t dir = esxdos_f_opendir(path);

  //  1  2  4  8
  // -? -a -l -1

  if (flags & 0x02) { listall = 1;               }
  if (flags & 0x04) { columns = 1; listlong = 1; }
  if (flags & 0x08) { columns = 1;               }

  if (dir == 0xff) {
    uint8_t f = esxdos_f_open(name, 'r');

    if (f != 0xff) {
      struct esxdos_stat st;

      if (esxdos_f_fstat(f, &st) < 0) {
        perror("ls");
      } else {
        if (listlong) {
          printf("F %6u %s %s\n", st.size, time, name);
        } else {
          printf("%-19s\n", name);
        }
      }

      esxdos_f_close(f);
    }
  } else {
    while (!esxdos_f_readdir(dir, &entry)) {
      struct esxdos_dirent_slice *info; // info -> attr, date, size
      col = COLOR_DEFAULT;
      type = 'F';

      if ((entry.dir[0] == '.') && (!listall)) continue;

      info = (struct esxdos_dirent_slice *)&entry.dir[ESXDOS_NAME_MAX+1];

      if (info->attr & ESXDOS_ATTR_DIR) {
        type = 'D';
        col = COLOR_BLUE;
      }

      size = info->size;

      if (listlong) {
        textcolor(COLOR_DEFAULT); printf("%c %6u %s ", type, info->size, time);
        textcolor(col);           printf("%s", entry.dir);
      } else {
        textcolor(col);           printf("%-19s", entry.dir);
      }

      if ((files++ % columns) == 0) {
        printf("\n");
      }
    }

    esxdos_f_close(dir);
  }

  if ((columns > 1) && (files % columns) == 0) {
    printf("\n");
  }

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

#endif // HAVE_DIVMMC
