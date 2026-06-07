<p align="center">
  <img src="docs/assets/astra-titlecard.png" alt="AstraRTOS" width="500"/>
</p>

![Status](https://img.shields.io/badge/status-in%20development-orange)
![Architecture](https://img.shields.io/badge/arch-ARM%20Cortex--M-blue)
![Target](https://img.shields.io/badge/target-STM32F429ZI-green)
![Toolchain](https://img.shields.io/badge/toolchain-arm--none--eabi--gcc-lightgrey)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)

AstraRTOS is a small kernel for ARM Cortex-M, being built from scratch. The current target is the STM32F429ZI Discovery board (Cortex-M4, ARMv7-M). It provides a preemptive, priority-scheduled environment for tasks running on a single MCU

## Features

- Priority-based preemptive scheduler
- Tasks, delays, counting semaphores, mutexes, software timers
- Dynamic memory
- GPIO, USART drivers, RCC clock control
- Runs on real hardware and under QEMU

## Documentation

The `docs/` directory contains the STM32F4 reference and programming manual PDFs, and `docs/blogs/` holds per subsystem walkthroughs and explanations

## Toolchain

- `arm-none-eabi-gcc`
- `make`
- `qemu-system-arm`
- `openocd`

On Debian / Ubuntu:

```bash
sudo apt install gcc-arm-none-eabi make qemu-system-arm openocd
```

## Build

```bash
make          # build/astra.elf, build/astra.bin
make qemu     # run under qemu-system-arm
make flash    # openocd flash via stlink
make clean
```

The application entry point is `app/main.c` and sample programs live in `samples/`, replacing the `app/main.c` with any sample and then rebuilding produces a runnable image

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for the commit message format, branch naming convention, and area prefixes. All changes must compile cleanly under `make clean && make` before submission.
