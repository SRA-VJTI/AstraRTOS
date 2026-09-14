#include "system_init.h"
#include "task.h"
#include "queue.h"
#include "heap.h"
#include "uart.h"
#include "mutex.h"

os_queue_t number_queue;
static os_mutex_t uart_mutex;

static void int_to_str(int value, char *buf) {
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

void producer1(void) {
    while(1) {
        char str[12];
        static int var1 = 0;
        os_mutex_take(&uart_mutex);
        uart_send_string(USART1_BASE, "Producer 1 Sent: ");
        int_to_str(var1, str);
        uart_send_string(USART1_BASE, str);
        uart_send_string(USART1_BASE, "\r\n");
        os_mutex_give(&uart_mutex);
        os_queue_send(&number_queue, &var1);
        var1++;
        os_delay(500);
    }
}

void producer2(void) {
    while(1) {
        char str[12];
        static int var2 = 0;
        os_mutex_take(&uart_mutex);
        uart_send_string(USART1_BASE, "Producer 2 Sent: ");
        int_to_str(var2, str);
        uart_send_string(USART1_BASE, str);
        uart_send_string(USART1_BASE, "\r\n");
        os_mutex_give(&uart_mutex);
        os_queue_send(&number_queue, &var2);
        var2++;
        os_delay(700);
    }
}

void consumer(void) {
    while(1) {
        int val;
        char str[12];
        os_queue_receive(&number_queue, &val);
        os_mutex_take(&uart_mutex);

        uart_send_string(USART1_BASE, "Consumer received: ");
        int_to_str(val, str);
        uart_send_string(USART1_BASE, str);
        uart_send_string(USART1_BASE, "\r\n");
        os_mutex_give(&uart_mutex);
    }
}

int main() {
    uart_config_t config = {.baud_rate = 9600,
                            .data_bits = 8,
                            .parity = UART_PARITY_DISABLE,
                            .stop_bits = UART_STOPBITS_1,
                            .mode = UART_MODE_TX_RX,
                            .irq_flags = UART_IRQ_RXNE};

    system_init();
    uart_init(USART1_BASE, &config);

    os_heap_init();
    os_queue_init(&number_queue, sizeof(int), 16);
    os_mutex_init(&uart_mutex);

    os_task_create(producer1, 2, 128);
    os_task_create(producer2, 2, 128);
    os_task_create(consumer, 2, 128);

    os_start();
    return 0;
}
