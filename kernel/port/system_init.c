#include "system_init.h"
#include "task.h"
#include "timer.h"
#define ICSR (*((volatile uint32_t *)0xE000ED04))

volatile uint32_t system_ticks = 0;

void system_init(void) {
    RCC_CR = RCC_CR | (1 << 16); // enable HSE clock using HSEON flag
#ifndef SIM_QEMU
    while(!(RCC_CR & (1 << 17)))
        ; // wait for ready flag to become 1 (HSERDY)
#endif
    FLASH_ACR = (5 << 0) | // wait 5 states for latency between flash and CPU (LATENCY[3:0])
                (1 << 8) | // enable prefetch [PRFTEN]
                (1 << 9) | // enable cache instructions [ICEN]
                (1 << 10); // enable cache data read    [DCEN]

    RCC_PLLCFGR = (8 << 0) |   // divide the HSE by 8 (PLLM)
                  (360 << 6) | // multiply by 360 (PLLN)
                  (0 << 16) |  // divide by 2 (PLLP)
                  (1 << 22);   // PLLSRC = HSE

    RCC_CR |= (1 << 24); // turn on PLL (PLLON)
#ifndef SIM_QEMU
    while(!(RCC_CR & (1 << 25)))
        ; // wait for ready flag (PLLRDY)
#endif
    RCC_CFGR = (0 << 4) |  // HPRE, AHB (180MHz)
               (5 << 10) | // PPRE1  APB1 / 4 (45 MHz)
               (4 << 13);  // PPRE2 APB2 / 2 (90 MHz)

    RCC_CFGR |= (2 << 0); // set PLL as system clock  (SW)
#ifndef SIM_QEMU
    while((RCC_CFGR & (3 << 2)) != (2 << 2))
        ; // wait till its active
#endif

    /* Value setup for SW:
       SWS = 0 -> HSI is used
       SWS = 1 -> HSE is used
       SWS = 2 -> PLL is used
    */
}

void systick_init(void) {
    STK_LOAD = (180000 - 1) << 0; // clock resets every 1ms

    STK_VAL = (0 << 0); // start from 0

    STK_CTRL = (1 << 2) | // processor clock used
               (1 << 1) | // asserts the SysTick exception request
               (1 << 0);  // enable counter
}

void SysTick_Handler(void) {
    system_ticks++; // update every 1ms
    os_decrement_blocked_tasks();
    os_timer_tick();
    ICSR |= (1 << 28); // trigger PendSV
}
