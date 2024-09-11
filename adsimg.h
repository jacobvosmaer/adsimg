#ifndef ADSIMG_H
#define ADSIMG_H

enum objecttype { OT_UNKNOWN, OT_MIX, OT_SOUND, OT_SAMPLE };
struct entry {
  unsigned char data[16];
  int offset, len;
  enum objecttype type;
};

struct floppy {
  unsigned char *data, *dataend;
  struct entry *toc, *tocend;
};

char *readfloppy(struct floppy *floppy);

#endif
