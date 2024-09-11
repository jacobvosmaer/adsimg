#ifndef ADSIMG_H
#define ADSIMG_H

#include <stdio.h>

enum objecttype { OT_UNKNOWN, OT_MIX, OT_SOUND, OT_SAMPLE };
struct entry {
  unsigned char data[16];
  int offset, len;
  enum objecttype type;
};

struct floppy {
  unsigned char data[1 << 21], *dataend;
  struct entry toc[3 * 256], *tocend;
};

/* On success readfloppy returns NULL. If non-null the return value is an error
 * message. */
char *readfloppy(struct floppy *floppy, FILE *in);

#endif
