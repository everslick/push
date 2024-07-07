#ifndef _PUSH_H_
#define _PUSH_H_

#if defined(KICKC) || defined(OSCAR64)
 #include "str.h"
#endif

#define SCRATCH_SIZE 128

extern char scratch[SCRATCH_SIZE];

#endif // _PUSH_H_
