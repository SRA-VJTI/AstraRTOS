# Samples

This directory contains sample applications demonstrating the features of AstraRTOS. 

You can run these samples either directly on the target **STM32 hardware** or via **QEMU** simulation.

## Running a sample
To run a specific sample, provide the path to sample in the `make` command
```bash
make SOURCE=path/to/sample/main.c
```

For example, to run the mutex sample:
```bash
make SOURCE=samples/mutex_basic/main.c
```

## QEMU
To simulate in QEMU
```bash
make clean
```
to clear the old build files first, then

```bash
make qemu SOURCE=path/to/sample/main.c
```