# Dynacord Sample Disk Splitter

This repository contains programs to work with Dynacord ADS sample
disks (also related instruments like ADD Two).

Compile the software with `make`. Use `toc < DISK_IMAGE` to list the
contents of a disk image. Use `split DIR IMAGE1 [IMAGE2 IMAGE3 ...]`
to extract samples from a series of disk images as WAV files. This
software does not support loop points and other metadata, you just get
the PCM sample data as WAVs.
