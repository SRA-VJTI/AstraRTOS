/* AstraRTOS - Startup code for STM32F429ZI */

.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.section .isr_vector, "a", %progbits    /* vector table */
.global _vector_table

_vector_table:
    .word _estack            /* 0x00: Initial stack pointer */
    .word Reset_Handler      /* 0x04: Reset - entry point */
    .word NMI_Handler        /* 0x08: Non-maskable interrupt */
    .word HardFault_Handler  /* 0x0C: Hard fault */
    .word MemManage_Handler  /* 0x10: Memory management fault */
    .word BusFault_Handler   /* 0x14: Bus fault */
    .word UsageFault_Handler /* 0x18: Usage fault */
    .word 0                  /* 0x1C: Reserved */
    .word 0                  /* 0x20: Reserved */
    .word 0                  /* 0x24: Reserved */
    .word 0                  /* 0x28: Reserved */
    .word SVC_Handler        /* 0x2C: Supervisor call */
    .word 0                  /* 0x30: Reserved */
    .word 0                  /* 0x34: Reserved */
    .word PendSV_Handler     /* 0x38: PendSV - context switch */
    .word SysTick_Handler    /* 0x3C: SysTick -  RTOS tick */
    .rept 37
    .word default            /* Default Handlers */
    .endr
    .word USART1_IRQHandler /* 0xD4: IRQ Handler for USART1 */
    .word USART2_IRQHandler /* 0xD8: IRQ Handler for USART2 */
    .word USART3_IRQHandler /* 0xDC: IRQ Handler for USART3 */


.section .text                  /* reset handler */
.global Reset_Handler
.type Reset_Handler, %function

Reset_Handler:
    /* Copy .data from the flash to SRAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata

copy_data:
    cmp r0, r1
    bge done_data
    ldr r3, [r2], #4
    str r3, [r0], #4
    b copy_data

done_data:
    /* Step 2: Zero out .bss */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0

zero_bss:
    cmp r0, r1
    bge done_bss
    str r2, [r0], #4
    b zero_bss

done_bss:
    /* call the main kernel */
    bl main

hang:
    b hang

/* to be defined in C later */
.weak NMI_Handler
.weak HardFault_Handler
.weak MemManage_Handler
.weak BusFault_Handler
.weak UsageFault_Handler
.weak SysTick_Handler
.weak default
.weak USART1_IRQHandler
.weak USART2_IRQHandler
.weak USART3_IRQHandler

.thumb_func
NMI_Handler:
.thumb_func
HardFault_Handler:
.thumb_func
MemManage_Handler:
.thumb_func
BusFault_Handler:
.thumb_func
UsageFault_Handler:
.thumb_func
SysTick_Handler:
.thumb_func
default:
.thumb_func
USART1_IRQHandler:
.thumb_func
USART2_IRQHandler:
.thumb_func
USART3_IRQHandler:
    b .

.extern os_current_task_ptr
.extern os_schedule_next_task
.global SVC_Handler
.type SVC_Handler, %function
SVC_Handler:
    ldr r3, =os_current_task_ptr
    ldr r1, [r3]
    ldr r0, [r1]
    ldmia r0!, {r4-r11}
    msr psp, r0
    mov r0, #0
    msr basepri, r0
    ldr r14, =0xFFFFFFFD
    bx r14
.global PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
    mrs r0, psp
    stmdb r0!, {r4-r11}
    ldr r1, =os_current_task_ptr
    ldr r2, [r1]
    str r0, [r2]
    push {r14}
    bl os_schedule_next_task
    pop {r14}
    ldr r1, =os_current_task_ptr
    ldr r2, [r1]
    ldr r0, [r2]
    ldmia r0!, {r4-r11}
    msr psp, r0
    bx r14