#include "system_init.h"
#include "uart.h"
#include "task.h"
#include "heap.h"
#include "sem.h"

#define UART_BASE USART1_BASE
#define MAX_CONCURRENT_USERS 2

static os_sem_t resource_sem;
static os_sem_t uart_sem;

static void uart_print_safe(const char *msg) {
    os_sem_take(&uart_sem);
    uart_send_string(UART_BASE, msg);
    os_sem_give(&uart_sem);
}

static void use_shared_resource(const char *tag) {
    uart_print_safe(tag);
    uart_print_safe(": waiting for resource\r\n");

    os_sem_take(&resource_sem);

    uart_print_safe(tag);
    uart_print_safe(": acquired resource\r\n");

    os_delay(300);

    uart_print_safe(tag);
    uart_print_safe(": releasing resource\r\n");

    os_sem_give(&resource_sem);
}

void task_a(void) {
    while(1) {
        use_shared_resource("[Task A - HIGH]");
        os_delay(100);
    }
}

void task_b(void) {
    while(1) {
        use_shared_resource("[Task B - LOW]");
        os_delay(100);
    }
}

void task_c(void) {
    while(1) {
        use_shared_resource("[Task C - MEDIUM]");
        os_delay(100);
    }
}

void task_d(void) {
    while(1) {
        use_shared_resource("[Task D - LOW]");
        os_delay(100);
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
    os_sem_init(&resource_sem, MAX_CONCURRENT_USERS, MAX_CONCURRENT_USERS);
    os_sem_init(&uart_sem, 1, 1);

    os_task_create(task_a, 3, 128);
    os_task_create(task_b, 1, 128);
    os_task_create(task_c, 2, 128);
    os_task_create(task_d, 1, 128);

    os_start();
    return 0;
}
