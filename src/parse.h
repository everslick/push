#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdint.h>

uint8_t parse_command(char *cmd, char **argv, uint8_t args);

const char *parse_dirname(const char *path);
const char *parse_basename(const char *path);
      char *parse_realpath(const char *path, char *unused);

uint8_t parse_optflags(uint8_t argc, char **argv, const char *optstr);
int8_t  parse_getopt(uint8_t nargc, char **nargv, const char *ostr);

extern char   *parse_optarg;
extern uint8_t parse_opterr;
extern uint8_t parse_optind;
extern uint8_t parse_optopt;
extern uint8_t parse_optreset;

#define getopt   parse_getopt
#define optarg   parse_optarg
#define opterr   parse_opterr
#define optind   parse_optind
#define optopt   parse_optopt
#define optreset parse_optreset

#endif // _PARSE_H_
