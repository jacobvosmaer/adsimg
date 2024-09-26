
#include "adsimg.h"
#include <stdio.h>
#include <string.h>

#define nelem(x) (sizeof(x) / sizeof(*(x)))
#define endof(x) ((x) + nelem(x))

#ifndef DEBUG
#define DEBUG 0
#endif

char *readfloppy(struct floppy *floppy, FILE *in) {
  unsigned char *p;
  struct entry *t;

  floppy->ndata = fread(floppy->data, 1, sizeof(floppy->data), in);
  if (ferror(in))
    return "read in failed";
  if (floppy->ndata == sizeof(floppy->data))
    return "input file too large to be a floppy";

  for (p = floppy->data + 0x200, t = floppy->toc;
       p + sizeof(t->desc) < floppy->data + floppy->ndata && p[10] != 0x7f &&
       t < endof(floppy->toc);
       p += sizeof(t->desc), t++) {
    memmove(t->desc, p, sizeof(t->desc));
    if (t > floppy->toc)
      t->offset = t[-1].offset + t[-1].len;
    else
      t->offset = 0x2e00;
    t->len = (((int)p[13] << 8) + (int)p[14]) * 512;
    if (p[11] == 0x00)
      t->type = OT_MIX;
    else if (p[11] == 0x01)
      t->type = OT_SOUND;
    else if (p[10] == 0x63)
      t->type = OT_SAMPLE;
    if (!(t == floppy->toc && t->type == OT_SAMPLE) &&
        memcmp(floppy->data + t->offset, p, 10) && DEBUG)
      return "name validation failed";
  }
  floppy->ntoc = t - floppy->toc;
  return 0;
}
