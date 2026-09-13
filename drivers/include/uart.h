#ifndef UART_H
#define UART_H

#include <stdint.h>

#define USART1_BASE 0x40011000
#define USART2_BASE 0x40004400
#define USART3_BASE 0x40004800
#define USART_SR(base) (*((volatile uint32_t *)((base) + 0x00)))
#define USART_DR(base) (*((volatile uint32_t *)((base) + 0x04)))
#define USART_BRR(base) (*((volatile uint32_t *)((base) + 0x08)))
#define USART_CR1(base) (*((volatile uint32_t *)((base) + 0x0C)))
#define USART_CR2(base) (*((volatile uint32_t *)((base) + 0x10)))
#define USART_CR3(base) (*((volatile uint32_t *)((base) + 0x14)))
#define NVIC_ISER1 (*(volatile uint32_t *)(0xE000E104))
#define ICSR (*((volatile uint32_t *)0xE000ED04))

#define TIMEOUT 1000

#define UART_PARITY_DISABLE 0
#define UART_PARITY_EVEN 1
#define UART_PARITY_ODD 2

#define UART_IRQ_NONE 0x00
#define UART_IRQ_RXNE (1 << 0) // RX Not Empty
#define UART_IRQ_TXE (1 << 1)  // TX Empty
#define UART_IRQ_TC (1 << 2)   // Transmission Complete

typedef enum {
    UART_STOPBITS_1 = 0,   // CR2[13:12] = 00
    UART_STOPBITS_0_5 = 1, // CR2[13:12] = 01
    UART_STOPBITS_2 = 2,   // CR2[13:12] = 10
    UART_STOPBITS_1_5 = 3  // CR2[13:12] = 11
} uart_stop_bits_t;

typedef enum {
    UART_MODE_RX = (1 << 2),
    UART_MODE_TX = (1 << 3),
    UART_MODE_TX_RX = ((1 << 3) | (1 << 2))
} uart_mode_t;

typedef struct {
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t parity;
    uart_stop_bits_t stop_bits;
    uart_mode_t mode;
    uint8_t irq_flags;
} uart_config_t;

void uart_init(uint32_t base, const uart_config_t *config);
void uart_send_char(uint32_t base, char c);
void uart_send_string(uint32_t base, char *str);
int uart_receive_char_polling(uint32_t base);
void uart_receive_string_polling(uint32_t base, char *buffer, int max_length);
int uart_receive_byte(uint32_t base, char *byte);

#endif