#ifndef ERR_H
#define ERR_H

#include <stdarg.h>

void err(int eval, const char *fmt, ...);
void errx(int eval, const char *fmt, ...);

#endif
