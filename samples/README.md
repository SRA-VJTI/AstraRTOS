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

## Hardware
To run the samples on hardware, use:
```bash 
make 
make flash
```

## QEMU
To simulate in QEMU, you need to modify the code.  

In your main() loop, comment out this line.  
```c
 system_init();  //comment out this line from the sample code
 ```
 
`system_init()` is responsible for configuring critical low-level subsystems, primarily the Reset and Clock Control (RCC) unit. This involves enabling internal/external oscillators (HSI/HSE), configuring the Phase-Locked Loops (PLL), and switching the system clock source.

Since QEMU focuses on CPU instruction emulation rather than register-accurate peripheral simulation, it does not fully simulate the RCC state transitions. Consequently, these hardware-ready flags are never set within the emulator, causing the program to trap itself in an infinite loop before the RTOS kernel or scheduler can boot.

To run the simulation in QEMU, use:
```bash
make qemu SOURCE=path/to/sample/main.c
```