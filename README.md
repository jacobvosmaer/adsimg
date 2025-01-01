# Dynacord Sample Disk Splitter

This repository contains programs to work with Dynacord ADS sample
disks (also related instruments like ADD Two).

Compile the software with `make`. Use `toc < DISK_IMAGE` to list the
contents of a disk image. Use `split DIR IMAGE1 [IMAGE2 IMAGE3 ...]`
to extract samples from a series of disk images as WAV files. This
software does not support loop points and other metadata, you just get
the PCM sample data as WAVs.

Example:

```
mkdir 112.000
./split 112.000 112.000*.img
```

Then in the 112.000 directory I see:

```
$ ls 112.000
01-BD_AMB.22_.wav 10-DESK_BELL_.wav 19-BD_GATED__.wav 28-TOM_13_GHD.wav 37-RIM_SPCE_G.wav
02-BD_AMB.KIC.wav 11-CUCKOO____.wav 20-BD_GATED_H.wav 29-TOM_12_RCK.wav 38-RIM_SPC_GH.wav
03-BD.NAT.20_.wav 12-BD_POWER__.wav 21-BD_ROCK___.wav 30-TOM_13_RCK.wav 39-JAZZ_CRASH.wav
04-SNARE_AMB..wav 13-SNARE_POWR.wav 22-SNARE_GTD_.wav 31-TOM_15_GTD.wav 40-DARK_RIDE_.wav
05-SNARE_A.SI.wav 14-TOM_12_PWR.wav 23-SNAR_GTD_H.wav 32-TOM_16_GTD.wav 41-TYMPANI_HI.wav
06-TOM_12_AMB.wav 15-TOM_13_PWR.wav 24-SNARE_ROCK.wav 33-TOM_15_GHD.wav 42-TYMPANI_ME.wav
07-TOM_13_AMB.wav 16-TOM_15_PWR.wav 25-TOM_12_GTD.wav 34-TOM_16_GHD.wav 43-TYMPANI_LO.wav
08-TOM_15_AMB.wav 17-TOM_16_PWR.wav 26-TOM_13_GTD.wav 35-TOM_15_RCK.wav 44-TEMPELGONG.wav
09-TOM_16_AMB.wav 18-CHINA_CRSH.wav 27-TOM_12_GHD.wav 36-TOM_16_RCK.wav 45-SHARP_GONG.wav
```

To compile the software you need a C compiler. There is a Makefile but
you don't have to use it, `cc -o split split.c adsimg.c` suffices. I
have developed these programs on macOS and they should compile on
Linux too. I have not tried Windows.

## Bugs

No extraction of sample metadata such as loop points and initial pitch. I also think the endings of the samples are wrong now. Samples are sometimes truncated a little before the ending.
