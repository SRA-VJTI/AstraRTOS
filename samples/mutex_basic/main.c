#include "system_init.h"
#include "uart.h"
#include "task.h"
#include "heap.h"
#include "mutex.h"

static os_mutex_t counter_mutex;
static volatile int shared_counter = 0;

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

void task1(void) {
    char num_str[12];

    while(1) {
        os_mutex_take(&counter_mutex);

        shared_counter++;
        uart_send_string(USART1_BASE, "Task1: counter = ");
        uint_to_str((uint32_t)shared_counter, num_str);
        uart_send_string(USART1_BASE, num_str);
        uart_send_string(USART1_BASE, "\r\n");

        os_mutex_give(&counter_mutex);
        os_delay(500);
    }
}

void task2(void) {
    char num_str[12];

    while(1) {
        os_mutex_take(&counter_mutex);

        shared_counter++;
        uart_send_string(USART1_BASE, "Task2: counter = ");
        uint_to_str((uint32_t)shared_counter, num_str);
        uart_send_string(USART1_BASE, num_str);
        uart_send_string(USART1_BASE, "\r\n");

        os_mutex_give(&counter_mutex);
        os_delay(700);
    }
}

int main(void) {
    system_init();
    uart_init(USART1_BASE);
    os_heap_init();

    os_mutex_init(&counter_mutex);

    os_task_create(task1, 2, 128);
    os_task_create(task2, 2, 128);

    os_start();
    return 0;
}