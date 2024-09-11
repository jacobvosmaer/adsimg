
#include "adsimg.h"
#include <err.h>
#include <stdio.h>

int main(void) {
  struct entry *t;
  struct floppy floppy = {0};
  char *err;

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
