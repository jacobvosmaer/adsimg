
#include "adsimg.h"
#include <err.h>
#include <stdio.h>
#include <string.h>

struct floppy floppy;

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

int main(void) {
  struct entry *t;
  int i;
  char *error = readfloppy(&floppy, stdin);
  if (error)
    errx(-1, "read floppy: %s", error);
  for (t = floppy.toc, i = 0; t < floppy.tocend; t++) {
    char *p, filename[18];
    FILE *f;
    enum { sampleheader = 512, sampletrailer = 512 };
    if (t->type != OT_SAMPLE)
      continue;
    i++;
    if (snprintf(filename, sizeof(filename), "%02d-%10.10s.wav", i, t->data) !=
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
    writewav(floppy.data + t->offset + sampleheader,
             t->len - sampleheader - sampletrailer, f);
    if (fclose(f))
      err(-1, "close %s", filename);
  }
}
