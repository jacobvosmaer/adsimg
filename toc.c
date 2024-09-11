
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *floppy, *floppyend;

enum objecttype { OT_UNKNOWN, OT_MIX, OT_SOUND, OT_SAMPLE };
struct entry {
  char name[11];
  int offset;
  enum objecttype type;
};
struct entry *toc, *tocend;

void readfloppy(void) {
  int floppysize = 0, floppycap = 0, tocsize;
  unsigned char *p;

  while (!feof(stdin)) {
    int n;
    while (n = fread(floppy + floppysize, 1, floppycap - floppysize, stdin), n)
      floppysize += n;
    if (ferror(stdin))
      err(-1, "read stdin");

    if (floppysize == floppycap) {
      floppycap = floppycap ? 2 * floppycap : 1;
      if (floppy = realloc(floppy, floppycap), !floppy)
        err(-1, "realloc floppy");
    }
  }
  floppyend = floppy + floppysize;

  for (p = floppy + 0x200, tocsize = 0; p < floppyend; p += 16, tocsize++) {
    struct entry *t;
    char endoftoc[] = "          \x7f\xff\x00\x00\x00\x00";
    if (!memcmp(p, endoftoc, sizeof(endoftoc) - 1))
      break;

    toc = realloc(toc, tocsize + 1);
    if (!toc)
      err(-1, "realloc toc");
    t = toc + tocsize;
    memmove(t->name, p, sizeof(t->name) - 1);
    t->name[sizeof(t->name) - 1] = 0;
    if (tocsize)
      t->offset = t[-1].offset + ((int)p[13] << 8) + (int)p[14];
    else
      t->offset = 0x2e00;
    if (p[11] == 0)
      t->type = OT_MIX;
    else if (p[11] == 1)
      t->type = OT_SOUND;
    else if (p[10] == 63)
      t->type = OT_SAMPLE;
  }
  tocend = toc + tocsize;
}

int main(void) {
  unsigned char *p;
  int offset = 0;

  readfloppy();
  for (p = floppy + 0x200; p < floppyend; p += 16) {
    char endoftoc[] = "          \x7f\xff\x00\x00\x00\x00";
    if (!memcmp(p, endoftoc, sizeof(endoftoc) - 1))
      break;
    if (*p & 0x80)
      printf("s %c", *p ^ 0x80);
    else
      printf("  %c", *p);
    printf("%9.9s", p + 1);
    printf(", p[10:15]=%02x %02x %02x %02x %02x, offset=%08x", p[10], p[11],
           p[12], p[13], p[14], 0x2e00 + offset);
    offset += (((int)p[13] << 8) + (int)p[14]) * 512;
    putchar('\n');

    if (p[12] || p[15]) {
      puts("unexpected non-zero byte");
      return 1;
    }
  }
}
