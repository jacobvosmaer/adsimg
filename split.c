
#include "adsimg.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void putle(unsigned x, int n, FILE *f) {
  for (; n > 0; n--, x >>= 8)
    fputc(x & 0xff, f);
}

/* Based on information from
 * https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html */
void writewav(unsigned char *data, int n, FILE *f) {
  int datasize = 8 + n, fmtsize = 8 + 16, wavesize = 4 + fmtsize + datasize,
      samplerate = 44100, samplebits = 16;
  fputs("RIFF", f);
  putle(wavesize, 4, f);
  fputs("WAVE", f);
  fputs("fmt ", f);
  putle(fmtsize - 8, 4, f);                   /* fmt chunk size */
  putle(1, 2, f);                             /* format 1 (PCM) */
  putle(1, 2, f);                             /* 1 channel */
  putle(samplerate, 4, f);                    /* sample rate 44.1kHz */
  putle((samplerate * samplebits) / 8, 4, f); /* data rate bytes/s */
  putle(samplebits / 8, 2, f);                /* bytes per sample */
  putle(samplebits, 2, f);                    /* bits per sample */
  fputs("data", f);
  putle(n, 4, f);
  /* ADS sample data is big-endian so we must convert the 16-bit samples to
   * little-endian now. */
  for (; n; n -= 2, data += 2) {
    fputc(data[1], f);
    fputc(data[0], f);
  }
}

struct floppy *floppy;
int nfloppy;

int main(int argc, char **argv) {
  struct entry *t;
  struct floppy *fl;
  int i, j;

  if (argc < 3)
    errx(-1, "usage: split DIR DISK [DISK...]");

  nfloppy = argc - 2;
  floppy = calloc(nfloppy, sizeof(*floppy));
  if (!floppy)
    errx(-1, "malloc");
  for (j = 0; j < nfloppy; j++) {
    char *error, *filename = argv[j + 2];
    FILE *f = fopen(filename, "rb");
    if (!f)
      err(-1, "open %s", filename);
    error = readfloppy(floppy + j, f);
    if (error)
      errx(-1, "readfloppy: %s", error);
    fclose(f);
  }

  if (chdir(argv[1]))
    err(-1, "chdir %s", argv[1]);

  for (fl = floppy, i = 0; fl < floppy + nfloppy; fl++) {
    for (t = fl->toc; t < fl->tocend; t++) {
      char *p, filename[18];
      FILE *f;
      enum { sampleheader = 512, sampletrailer = 512 };
      if (t->type != OT_SAMPLE)
        continue;
      i++;
      if (snprintf(filename, sizeof(filename), "%02d-%10.10s.wav", i,
                   t->data) != sizeof(filename) - 1)
        errx(-1, "filename size error");
      for (p = filename; *p; p++) {
        *p &= 0x7f;
        if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
              (*p >= '0' && *p <= '9') || *p == '-' || *p == '.'))
          *p = '_';
      }
      if (f = fopen(filename, "wb"), !f)
        err(-1, "fopen %s", filename);
      writewav(fl->data + t->offset + sampleheader,
               t->len - sampleheader - sampletrailer, f);
      if (fclose(f))
        err(-1, "close %s", filename);
    }
  }
}
