#include "system_init.h"
#include "uart.h"
#include "task.h"
#include "heap.h"
#include "stats.h"
#include "mutex.h"

#define UART_BASE USART1_BASE

static os_mutex_t uart_mutex;

static void uint_to_str(uint32_t value, char *buf) {
    char tmp[12];
    int i = 0;
    if(value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while(value > 0) {
        tmp[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    int j = 0;
    while(i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

void stats_task(void) {
    char buf[64];
    os_task_stats_t stats;
    while(1) {
        os_mutex_take(&uart_mutex);
        uart_send_string(UART_BASE, "Task Stats \r\n");
        os_mutex_give(&uart_mutex);
        uint32_t total = os_get_task_count();
        for(uint32_t i = 0; i < total; i++) {
            if(os_task_get_stats(i, &stats) == 0) {
                os_mutex_take(&uart_mutex);
                uart_send_string(UART_BASE, "Task ");
                uint_to_str(stats.task_num, buf);
                uart_send_string(UART_BASE, buf);
                uart_send_string(UART_BASE, ": runs=");
                uint_to_str(stats.run_count, buf);
                uart_send_string(UART_BASE, buf);
                uart_send_string(UART_BASE, ", stack_water=");
                uint_to_str(stats.stack_high_water, buf);
                uart_send_string(UART_BASE, buf);
                uart_send_string(UART_BASE, "/");
                uint_to_str(stats.stack_size, buf);
                uart_send_string(UART_BASE, buf);
                uart_send_string(UART_BASE, "\r\n");
                os_mutex_give(&uart_mutex);
            }
        }
        os_delay(1000);
    }
}

void addition_task(void) {
    static uint32_t sum = 0;
    char buf[16];
    while(1) {
        sum += 5;
        os_mutex_take(&uart_mutex);
        uart_send_string(UART_BASE, "Addition Task: sum = ");
        uint_to_str(sum, buf);
        uart_send_string(UART_BASE, buf);
        uart_send_string(UART_BASE, "\r\n");
        os_mutex_give(&uart_mutex);
        os_delay(1000);
    }
}

int main(void) {
    uart_config_t config = {.baud_rate = 9600,
                            .data_bits = 8,
                            .parity = UART_PARITY_DISABLE,
                            .stop_bits = UART_STOPBITS_1,
                            .mode = UART_MODE_TX_RX,
                            .irq_flags = UART_IRQ_RXNE};

    system_init();
    uart_init(UART_BASE, &config);

    os_heap_init();
    os_mutex_init(&uart_mutex);
    os_task_create(stats_task, 3, 256);
    os_task_create(addition_task, 2, 128);
    os_start();
    return 0;
}
