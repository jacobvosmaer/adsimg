
#include "adsimg.h"
#include "err.h"
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
void writewav(const struct iovec *iov, int iovcnt, int samplerate, FILE *f) {
  int samplebits = 16, i;
  int64_t datasize, fmtsize, wavesize, n;
  for (i = 0, n = 0; i < iovcnt; i++) {
    assert(iov[i].iov_len <= UINT32_MAX - n);
    n += iov[i].iov_len;
  }
  assert(n <= UINT32_MAX);
  datasize = 8 + n;
  fmtsize = 8 + 16;
  wavesize = 4 + fmtsize + datasize;
  assert(wavesize <= UINT32_MAX);
  fputs("RIFF", f);
  putle(wavesize, 4, f);
  fputs("WAVE", f);
  fputs("fmt ", f);
  putle(fmtsize - 8, 4, f); /* fmt chunk size */
  putle(1, 2, f);           /* format 1 (PCM) */
  putle(1, 2, f);           /* 1 channel */
  putle(samplerate, 4, f);
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
  int samplerate; /* sample rate in samples/s */
  int len; /* sample data remaining in bytes. Used for bookkeeping while parsing
              floppy, should be 0 when done. */
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
      struct iovec *newiov;
      enum { sampleheader = 512, sampleid = 11 };
      unsigned char *chunkstart = fl->data + t->offset,
                    *chunkend = chunkstart + t->len;
      int merge;
      if (t->type != OT_SAMPLE)
        continue;
      if (!(chunkstart >= fl->data && chunkstart <= chunkend &&
            chunkend <= fl->data + fl->ndata)) {
        fprintf(stderr, "warning: skiping entry with invalid bounds: %10.10s\n",
                t->desc);
        continue;
      }
      s = sample + nsample++;
      assert(s < endof(sample));
      memmove(s->desc, t->desc, sizeof(s->desc));
      s->samplerate = chunkstart[14] >> 4 == 0x3 ? 44100 : 22050;
      /* The header tells us the sample size in 16-bit samples. For our
       * calculation we care about the size in bytes so we multiply by 2. */
      s->len = 2 * (((int)chunkstart[20] << 16) + ((int)chunkstart[18] << 8) +
                    ((int)chunkstart[19] << 0));
      merge = s > sample && s[0].desc[sampleid] == s[-1].desc[sampleid];
      if (merge) { /* don't create new sample */
        s--;
        nsample--;
      }
      s->iov = Realloc(s->iov, ++(s->iovcnt), sizeof(*(s->iov)));
      newiov = s->iov + s->iovcnt - 1;
      newiov->iov_base = (char *)chunkstart + (merge ? 0 : sampleheader);
      newiov->iov_len = (char *)chunkend - newiov->iov_base;
      if (s->len < newiov->iov_len)
        newiov->iov_len = s->len;
      s->len -= newiov->iov_len;
    }
  }

  for (s = sample; s < sample + nsample; s++) {
    char filename[18];
    if (s->len) {
      fprintf(stderr, "missing sample data for sample %10.10s, skipping\n",
              s->desc);
      continue;
    }
    if (snprintf(filename, sizeof(filename), "%02ld-%10.10s.wav",
                 s - sample + 1, s->desc) != sizeof(filename) - 1)
      errx(-1, "filename size error");
    sanitizefilename(filename);
    if (f = fopen(filename, "wb"), !f)
      err(-1, "fopen %s", filename);
    writewav(s->iov, s->iovcnt, s->samplerate, f);
    if (fclose(f))
      err(-1, "close %s", filename);
  }
  return 0;
}
