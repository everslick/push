#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <stdint.h>

char *fileio_getcwd(char *buf, uint8_t size);

int8_t fileio_version(char *buf, uint8_t size);

int8_t fileio_chdir(const char *dir);
int8_t fileio_mkdir(const char *dir);
int8_t fileio_rmdir(const char *dir);

int8_t fileio_ls(uint8_t flags, const char *path);
void fileio_mount(const char *dev, const char *dir);

void fileio_error(const char *cmd);

/* List of I/O error codes:
**
** error-code   BASIC error
** ----------   -----------
**       1  =   too many files
**       2  =   file open
**       3  =   file not open
**       4  =   file not found
**       5  =   device not present
**       6  =   not input-file
**       7  =   not output-file
**       8  =   missing file-name
**       9  =   illegal device-number
**
**      10  =   STOP-key pushed
**      11  =   general I/O-error
**
**      -1  =   [open/write failed]
**      -2  =   [read status failed]
*/

#endif // _FILEIO_H_
