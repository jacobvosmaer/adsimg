
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char *readfloppy(struct floppy *floppy) {
  int datasize = 0, datacap = 0, tocsize;
  unsigned char *p;
  struct entry *tt;

  while (!feof(stdin)) {
    int n;
    while (n = fread(floppy->data + datasize, 1, datacap - datasize, stdin),
           n > 0)
      datasize += n;
    if (ferror(stdin))
      return "read stdin failed";

    if (datasize == datacap) {
      datacap = datacap ? 2 * datacap : 1;
      if (floppy->data = realloc(floppy->data, datacap * sizeof(*floppy->data)),
          !floppy->data)
        return "realloc floppy->data failed";
    }
  }
  floppy->dataend = floppy->data + datasize;

  for (p = floppy->data + 0x200, tocsize = 0;
       p < floppy->dataend - sizeof(floppy->toc->data) && p[10] != 0x7f;
       p += sizeof(floppy->toc->data), tocsize++) {
    struct entry *t;

    if (floppy->toc =
            realloc(floppy->toc, (tocsize + 1) * sizeof(*(floppy->toc))),
        !floppy->toc)
      return "realloc toc failed";
    t = floppy->toc + tocsize;
    memmove(t->data, p, sizeof(t->data));
    if (t > floppy->toc)
      t->offset = t[-1].offset + t[-1].len;
    else
      t->offset = 0x2e00;
    if (memcmp(floppy->data + t->offset, p, 10))
      return "name validation failed";
    t->len = (((int)p[13] << 8) + (int)p[14]) * 512;
    if (p[11] == 0x00)
      t->type = OT_MIX;
    else if (p[11] == 0x01)
      t->type = OT_SOUND;
    else if (p[10] == 0x63)
      t->type = OT_SAMPLE;
  }
  floppy->tocend = floppy->toc + tocsize;
  return 0;
}

int main(void) {
  struct entry *t;
  struct floppy floppy = {0};
  char *err;
  int offset = 0;

  if (err = readfloppy(&floppy), err)
    errx(-1, "read floppy: %s", err);

  for (t = floppy.toc; t < floppy.tocend; t++) {
    char *types[] = {"???", "mix", "snd", "sam"};
    unsigned char *p = t->data;
    printf("%s %c%9.9s", types[t->type], *p & 0x7f, p + 1);
    printf(", p[10:15]=%02x %02x %02x %02x %02x, offset=%08x len=%d", p[10],
           p[11], p[12], p[13], p[14], t->offset, t->len);
    putchar('\n');

    if (p[12] || p[15]) {
      puts("unexpected non-zero byte");
      return 1;
    }
  }
}
