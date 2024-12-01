
#include "err.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void error(int errnosuffix, int eval, const char *fmt, va_list ap) {
  fflush(stdout);
  fputs("error: ", stderr);
  vfprintf(stderr, fmt, ap);
  if (errnosuffix)
    fprintf(stderr, ": %s", strerror(errno));
  fputc('\n', stderr);
  exit(eval);
}

void err(int eval, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  error(1, eval, fmt, ap);
  va_end(ap);
}

void errx(int eval, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  error(0, eval, fmt, ap);
  va_end(ap);
}
