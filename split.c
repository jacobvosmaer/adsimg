
#include "adsimg.h"
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define nelem(x) (sizeof(x) / sizeof(*(x)))
#define endof(x) ((x) + nelem(x))
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()

void putle(unsigned x, int n, FILE *f) {
  for (; n > 0; n--, x >>= 8)
    fputc(x & 0xff, f);
}

struct iovec {
  char *iov_base;
  size_t iov_len;
};

/* Based on information from
 * https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html */
void writewav(const struct iovec *iov, int iovcnt, FILE *f) {
  int datasize, fmtsize, wavesize, samplerate = 44100, samplebits = 16, i, n;
  for (i = 0, n = 0; i < iovcnt; i++)
    n += iov[i].iov_len;
  datasize = 8 + n;
  fmtsize = 8 + 16;
  wavesize = 4 + fmtsize + datasize;
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
  for (; iovcnt; iovcnt--, iov++) {
    for (i = 0; i < iov->iov_len; i += 2) {
      fputc(iov->iov_base[i + 1], f);
      fputc(iov->iov_base[i], f);
    }
  }
}

void *Realloc(void *p, int count, int size) {
  assert(count >= 0);
  assert(size >= 0);
  if (size)
    assert(count < SIZE_MAX / size);
  p = realloc(p, count * size);
  assert(p);
  return p;
}

struct floppy *floppy;
int nfloppy;

struct sample {
  char desc[sizeof(floppy->toc->desc)];
  struct iovec *iov;
  int iovcnt;
} sample[256];
int nsample;

void sanitizefilename(char *filename) {
  char *p;
  for (p = filename; *p; p++) {
    *p &= 0x7f;
    if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
          (*p >= '0' && *p <= '9') || *p == '-' || *p == '.'))
      *p = '_';
  }
}

int main(int argc, char **argv) {
  struct entry *t;
  struct floppy *fl;
  int j;
  struct sample *s;
  FILE *f;

  if (argc < 3)
    errx(-1, "usage: split DIR DISK [DISK...]");

  nfloppy = argc - 2;
  if (floppy = calloc(nfloppy, sizeof(*floppy)), !floppy)
    errx(-1, "malloc");
  for (j = 0; j < nfloppy; j++) {
    char *error, *filename = argv[j + 2];
    if (f = fopen(filename, "rb"), !f)
      err(-1, "open %s", filename);
    if (error = readfloppy(floppy + j, f), error)
      errx(-1, "readfloppy: %s", error);
    fclose(f);
  }

  if (chdir(argv[1]))
    err(-1, "chdir %s", argv[1]);

  for (fl = floppy, nsample = 0; fl < floppy + nfloppy; fl++) {
    for (t = fl->toc; t < fl->toc + fl->ntoc; t++) {
      struct iovec *iov;
      enum { sampleheader = 512, sampletrailer = 512, sampleid = 11 };
      char *chunkstart = (char *)fl->data + t->offset;
      int merge;
      if (t->type != OT_SAMPLE)
        continue;
      s = sample + nsample++;
      assert(s < endof(sample));
      memmove(s->desc, t->desc, sizeof(s->desc));
      merge = s > sample && s[0].desc[sampleid] == s[-1].desc[sampleid];
      if (merge) { /* don't create new sample */
        s--;
        nsample--;
      }
      s->iov = Realloc(s->iov, ++(s->iovcnt), sizeof(*(s->iov)));
      iov = s->iov + s->iovcnt - 1;
      iov->iov_base = chunkstart + sampleheader;
      iov->iov_len = chunkstart + t->len - sampletrailer - iov->iov_base;
      if (merge) { /* previous chunk has no trailer and current has no header */
        assert(s->iovcnt > 1);
        iov[-1].iov_len += sampletrailer;
        iov[0].iov_base -= sampleheader;
      }
    }
  }

  for (s = sample; s < sample + nsample; s++) {
    char filename[18];
    if (snprintf(filename, sizeof(filename), "%02ld-%10.10s.wav",
                 s - sample + 1, s->desc) != sizeof(filename) - 1)
      errx(-1, "filename size error");
    sanitizefilename(filename);
    if (f = fopen(filename, "wb"), !f)
      err(-1, "fopen %s", filename);
    writewav(s->iov, s->iovcnt, f);
    if (fclose(f))
      err(-1, "close %s", filename);
  }
  return 0;
}
