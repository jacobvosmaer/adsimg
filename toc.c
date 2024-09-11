
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *floppy, *floppyend;

enum objecttype { OT_UNKNOWN, OT_MIX, OT_SOUND, OT_SAMPLE };
struct entry {
  unsigned char data[16];
  int offset;
  enum objecttype type;
};
struct entry *toc, *tocend;

void readfloppy(void) {
  int floppysize = 0, floppycap = 0, tocsize;
  unsigned char *p;
  struct entry *tt;

  while (!feof(stdin)) {
    int n;
    while (n = fread(floppy + floppysize, 1, floppycap - floppysize, stdin), n)
      floppysize += n;
    if (ferror(stdin))
      err(-1, "read stdin");

    if (floppysize == floppycap) {
      floppycap = floppycap ? 2 * floppycap : 1;
      if (floppy = realloc(floppy, floppycap * sizeof(*floppy)), !floppy)
        err(-1, "realloc floppy");
    }
  }
  floppyend = floppy + floppysize;

  for (p = floppy + 0x200, tocsize = 0; p < floppyend;
       p += sizeof(toc->data), tocsize++) {
    struct entry *t;
    if (p[10] == 0x7f)
      break;

    if (toc = realloc(toc, (tocsize + 1) * sizeof(*toc)), !toc)
      err(-1, "realloc toc");
    t = toc + tocsize;
    memmove(t->data, p, sizeof(t->data));
    if (t > toc)
      t->offset = t[-1].offset +
                  (((int)t[-1].data[13] << 8) + (int)t[-1].data[14]) * 512;
    else
      t->offset = 0x2e00;
    if (p[11] == 0x00)
      t->type = OT_MIX;
    else if (p[11] == 0x01)
      t->type = OT_SOUND;
    else if (p[10] == 0x63)
      t->type = OT_SAMPLE;
  }
  tocend = toc + tocsize;
}

int main(void) {
  struct entry *t;
  int offset = 0;

  readfloppy();
  for (t = toc; t < tocend; t++) {
    char *types[] = {"???", "mix", "snd", "sam"};
    unsigned char *p = t->data;
    printf("%s %c%9.9s", types[t->type], *p & 0x7f, p + 1);
    printf(", p[10:15]=%02x %02x %02x %02x %02x, offset=%08x", p[10], p[11],
           p[12], p[13], p[14], t->offset);
    putchar('\n');

    if (p[12] || p[15]) {
      puts("unexpected non-zero byte");
      return 1;
    }
  }
}
