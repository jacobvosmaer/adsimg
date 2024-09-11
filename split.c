
#include "adsimg.h"
#include <err.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  struct entry *t;
  struct floppy floppy = {0};
  int i;
  char *error = readfloppy(&floppy, stdin);
  if (error)
    errx(-1, "read floppy: %s", error);
  for (t = floppy.toc, i = 0; t < floppy.tocend; t++) {
    char *p, filename[18], firstsample[] = "r. Disk operating system";
    FILE *f;
    int start, n;
    if (t->type != OT_SAMPLE)
      continue;
    i++;
    if (snprintf(filename, sizeof(filename), "%02d-%10.10s.s16", i, t->data) !=
        sizeof(filename) - 1)
      errx(-1, "filename size error");

    for (p = filename; *p; p++) {
      *p &= 0x7f;
      if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
            (*p >= '0' && *p <= '9') || *p == '-' || *p == '.'))
        *p = '_';
    }
    if (f = fopen(filename, "wb"), !f)
      err(-1, "fopen %s", filename);
    start = 36;
    if (!memcmp(floppy.data + t->offset + start, firstsample,
                sizeof(firstsample) - 1))
      start += 476;
    n = t->len - start;
    if (fwrite(floppy.data + t->offset + start, 1, n, f) != n)
      err(-1, "write sample data to %s", filename);
    if (fclose(f))
      err(-1, "close %s", filename);
  }
}
