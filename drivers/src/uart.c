#include "uart.h"
#include "rcc.h"
#include "gpio.h"
#include "system_init.h"

#define RINGBUFFER_SIZE 128

typedef struct {
    char buffer[RINGBUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} uart_ringbuffer_t;

static uart_ringbuffer_t ring_buffer[3] = {
    {.head = 0, .tail = 0}, {.head = 0, .tail = 0}, {.head = 0, .tail = 0}};

static int get_uart_idx(uint32_t base) {
    if(base == USART1_BASE) {
        return 0;
    }
    else if(base == USART2_BASE) {
        return 1;
    }
    else if(base == USART3_BASE) {
        return 2;
    }
    else {
        return -1;
    }
}

static void uart_clock_enable(uint32_t base) {
    if(base == USART1_BASE) {
        rcc_enable_uart(USART1_EN);
    }
    else if(base == USART2_BASE) {
        rcc_enable_uart(USART2_EN);
    }
    else if(base == USART3_BASE) {
        rcc_enable_uart(USART3_EN);
    }
}

void uart_set_mode(uint32_t base, uart_mode_t mode) {
    USART_CR1(base) &= ~((1 << 3) | (1 << 2));
    USART_CR1(base) |= mode;
}

void uart_enable(uint32_t base) {
    USART_CR1(base) |= (1 << 13);
}

void uart_disable(uint32_t base) {
    USART_CR1(base) &= ~(1 << 13);
}

static void uart_gpio_config(uint32_t base) {
    gpio_config_t uart_config = {.mode = GPIO_MODE_AF,
                                 .otype = GPIO_OTYPE_PP,
                                 .ospeed = GPIO_OSPEED_HIGH,
                                 .pull = GPIO_PULL_UP,
                                 .af = 7};
    if(base == USART1_BASE) {
        gpio_init(GPIOA_BASE, 9, &uart_config);
        gpio_init(GPIOA_BASE, 10, &uart_config);
    }
    else if(base == USART2_BASE) {
        gpio_init(GPIOA_BASE, 2, &uart_config);
        gpio_init(GPIOA_BASE, 3, &uart_config);
    }
    else if(base == USART3_BASE) {
        gpio_init(GPIOB_BASE, 10, &uart_config);
        gpio_init(GPIOB_BASE, 11, &uart_config);
    }
}

void uart_set_data_bits(uint32_t base, uint8_t data_bits) {
    if(data_bits == 9) {
        USART_CR1(base) |= (1 << 12);
    }
    else if(data_bits == 8) {
        USART_CR1(base) &= ~(1 << 12);
    }
}

void uart_set_parity(uint32_t base, uint8_t parity) {
    if(parity == UART_PARITY_DISABLE) {
        USART_CR1(base) &= ~((1 << 10) | (1 << 9));
    }
    else if(parity == UART_PARITY_EVEN) {
        USART_CR1(base) &= ~(1 << 9);
        USART_CR1(base) |= (1 << 10);
    }
    else if(parity == UART_PARITY_ODD) {
        USART_CR1(base) |= (1 << 10) | (1 << 9);
    }
}

void uart_set_stop_bits(uint32_t base, uart_stop_bits_t stop_bits) {
    USART_CR2(base) &= ~(3 << 12);
    USART_CR2(base) |= ((stop_bits & 0x03) << 12);
}

void uart_set_baud_rate(uint32_t base, uint32_t baudrate) {
    uint32_t pclk = 0;
    if(base == USART2_BASE || base == USART3_BASE) {
        pclk = rcc_get_apb1_freq();
    }
    else if(base == USART1_BASE) {
        pclk = rcc_get_apb2_freq();
    }
    USART_BRR(base) = (pclk + (baudrate / 2)) / baudrate;
}

void uart_interrupt_config(uint32_t base, uint8_t irq_flags) {
    if(irq_flags == UART_IRQ_NONE)
        return;
    if(irq_flags & UART_IRQ_RXNE) {
        USART_CR1(base) |= (1 << 5);
    }
    if(irq_flags & UART_IRQ_TXE) {
        USART_CR1(base) |= (1 << 7);
    }
    switch(base) {
    case USART1_BASE:
        NVIC_ISER1 |= (1 << 5);
        break;
    case USART2_BASE:
        NVIC_ISER1 |= (1 << 6);
        break;
    case USART3_BASE:
        NVIC_ISER1 |= (1 << 7);
        break;
    default:
        break;
    }
}

void uart_init(uint32_t base, const uart_config_t *config) {
    uart_clock_enable(base);
    uart_gpio_config(base);

    uart_set_baud_rate(base, config->baud_rate);
    uart_set_data_bits(base, config->data_bits);
    uart_set_parity(base, config->parity);
    uart_set_stop_bits(base, config->stop_bits);
    uart_set_mode(base, config->mode);

    uart_interrupt_config(base, config->irq_flags);
    uart_enable(base);
}

void uart_send_char(uint32_t base, char c) {
    while(!(USART_SR(base) & (1 << 7)))
        ;                        // Checking bit 7 of SR
    USART_DR(base) = (uint8_t)c; // Write to DR
}

// After writing to DR the TXE flag (bit 7) resets back

void uart_send_string(uint32_t base, char *str) {
    while(*str) {
        uart_send_char(base, *str++);
    }
}

int uart_receive_char_polling(uint32_t base) {
    USART_CR1(base) &= ~(1 << 5);

    uint32_t initial_ticks = system_ticks;

    while(!(USART_SR(base) & (1 << 5))) { // Checking bit 5 of SR
        if(system_ticks - initial_ticks >= TIMEOUT) {
            return -1; // Timeout if nothing is received (after 1 sec)
        }
    }
    int data = (int)(USART_DR(base) & 0xFF);
    USART_CR1(base) |= (1 << 5);

    return data;
}

void uart_receive_string_polling(uint32_t base, char *buffer, int max_length) {
    int i = 0;
    int c;

    while(i < max_length - 1) {              // Iterate until (max - 1)
        c = uart_receive_char_polling(base); // Receive characters

        if(c == -1)
            break; // Break if timeout
        if(c == '\r' || c == '\n')
            break; // Break if carriage return or newline

        buffer[i++] = (char)c; // Put in buffer
    }

    buffer[i] = '\0'; // Null terminator
}

int uart_receive_byte(uint32_t base, char *byte) {
    int idx = get_uart_idx(base);
    if(idx == -1) {
        return 0;
    }
    if(ring_buffer[idx].head == ring_buffer[idx].tail) {
        return 0;
    }
    *byte = ring_buffer[idx].buffer[ring_buffer[idx].tail];
    ring_buffer[idx].tail = (ring_buffer[idx].tail + 1) % RINGBUFFER_SIZE;

    return 1;
}

static void handle_irq(uint32_t base, int idx) {
    if(USART_SR(base) & (1 << 5)) {
        char recd_byte = (char)USART_DR(base);
        uint32_t next_head = (ring_buffer[idx].head + 1) % RINGBUFFER_SIZE;
        if(next_head != ring_buffer[idx].tail) {
            ring_buffer[idx].buffer[ring_buffer[idx].head] = recd_byte;
            ring_buffer[idx].head = next_head;

            ICSR |= (1 << 28);
        }
    }
}

void USART1_IRQHandler(void) {
    handle_irq(USART1_BASE, 0);
}

void USART2_IRQHandler(void) {
    handle_irq(USART2_BASE, 1);
}

void USART3_IRQHandler(void) {
    handle_irq(USART3_BASE, 2);
}