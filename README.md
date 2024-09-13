# Dynacord Sample Disk Splitter

This repository contains programs to work with Dynacord ADS sample
disks (also related instruments like ADD Two).

Compile the software with `make`. Use `toc < DISK_IMAGE` to list the
contents of a disk image. Use `split DIR IMAGE1 [IMAGE2 IMAGE3 ...]`
to extract samples from a series of disk images as WAV files. This
software does not support loop points and other metadata, you just get
the PCM sample data as WAVs.

To compile the software you need a C compiler. There is a Makefile but
you don't have to use it, `cc -o split split.c adsimg.c` suffices. I
have developed these programs on macOS and they should compile on
Linux too. I have not tried Windows.
