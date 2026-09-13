#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#define HCLK_FREQ 180000000

#include <stdint.h>

#define RCC_BASE 0x40023800
#define RCC_CR (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_PLLCFGR (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_CFGR (*(volatile uint32_t *)(RCC_BASE + 0x08))
#define FLASH_ACR (*(volatile uint32_t *)(0x40023C00))

#define SYSTICK_BASE 0xE000E010
#define STK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define STK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define STK_VAL (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))
#define STK_CALIB (*(volatile uint32_t *)(SYSTICK_BASE + 0x0C))

extern volatile uint32_t system_ticks;

void system_init(void);
void systick_init(void);
void SysTick_Handler(void);

uint32_t rcc_get_apb1_freq(void);
uint32_t rcc_get_apb2_freq(void);

#endif
