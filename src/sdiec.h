#ifndef _SDIEC_H_
#define _SDIEC_H_

#include <stdint.h>

char *sdiec_getcwd(char buf[], uint8_t size);

int8_t sdiec_version(char buf[], uint8_t size);

int8_t sdiec_chdir(const char *dir);
int8_t sdiec_mkdir(const char *dir);
int8_t sdiec_rmdir(const char *dir);

void sdiec_error(const char *cmd);

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

#endif // _SDIEC_H_
