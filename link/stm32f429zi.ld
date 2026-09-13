/* AstraRTOS - Linker script for stm32f429zi */

_estack = ORIGIN(SRAM) + LENGTH(SRAM);  /* start of stack (downwards growth) */
_Min_stack_size = 1024;  /* stack size reserved for the msp */

MEMORY
{
    FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
    SRAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 192K      /* 112 + 16 + 64 */
    CCMRAM (rw)  : ORIGIN = 0x10000000, LENGTH = 64K
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >SRAM AT> FLASH   /* stored at flash ran at sram */

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >SRAM
    _sheap = ALIGN(_ebss,32);
    _eheap = _estack - _Min_stack_size;
}