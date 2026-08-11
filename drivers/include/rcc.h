#ifndef RCC_H
#define RCC_H

#include <stdint.h>

/* EN defines prefered not be used in
 * kernel, needs updating in uart rn
 */

#define GPIOA_EN 0
#define GPIOB_EN 1
#define GPIOC_EN 2
#define GPIOD_EN 3
#define GPIOE_EN 4
#define GPIOF_EN 5
#define GPIOG_EN 6
#define GPIOH_EN 7
#define GPIOI_EN 8
#define GPIOJ_EN 9
#define GPIOK_EN 10

#define USART1_EN 1
#define USART2_EN 2
#define USART3_EN 3

#define TIM1_BASE 0x40010000
#define TIM2_BASE 0x40000000
#define TIM3_BASE 0x40000400
#define TIM4_BASE 0x40000800
#define TIM5_BASE 0x40000C00
#define TIM8_BASE 0x40010400
#define TIM9_BASE 0x40014000
#define TIM10_BASE 0x40014400
#define TIM11_BASE 0x40014800
#define TIM12_BASE 0x40001800
#define TIM13_BASE 0x40001C00
#define TIM14_BASE 0x40002000

#define RCC_BASE 0x40023800

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30)) // For GPIOA-GPIOI, DMA1,2
#define RCC_AHB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x34)) // For USB, RNG
#define RCC_AHB3ENR (*(volatile uint32_t *)(RCC_BASE + 0x38)) // External Memory (FSMC/FMC)
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40)) // For UART2,3 , I2C
#define RCC_APB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x44)) // For UART1, ADC, SPI1
#define RCC_CSR (*(volatile uint32_t *)(RCC_BASE + 0x74))     // Reset Flags and LSI Enable

void rcc_enable_gpio(uint8_t port);
void rcc_disable_gpio(uint8_t port);
void rcc_enable_uart(uint8_t uart);
void rcc_disable_uart(uint8_t uart);
void rcc_enable_timer(uint32_t timer);
uint32_t rcc_get_apb1_freq(void);
uint32_t rcc_get_apb2_freq(void);

#endif
