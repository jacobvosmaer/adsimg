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
  unsigned char
      data[1 << 21]; /* A floppy is at most 1.44 MB, i.e. less than 2MB */
  unsigned char *dataend;
  struct entry
      toc[3 * 256]; /* I have seen 3 object types in the table of contents, each
                       with an 8-bit index counter so the worst case table of
                       contents size seems to be 3 * 256. */
  struct entry *tocend;
};

/* On success readfloppy returns NULL. If non-null the return value is an error
 * message. */
char *readfloppy(struct floppy *floppy, FILE *in);

#endif
