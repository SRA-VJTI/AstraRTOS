#include "system_init.h"
#include "rcc.h"
#include "gpio.h"
#include "task.h"
#include "uart.h"
#include "heap.h"

void task1(void) {
    gpio_set_mode(GPIOG_BASE, 13, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_write(GPIOG_BASE, 13, 1);
        os_delay(500);
        gpio_write(GPIOG_BASE, 13, 0);
        os_delay(500);
    }
}

void task2(void) {
    gpio_set_mode(GPIOG_BASE, 14, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_write(GPIOG_BASE, 14, 1);
        os_delay(700);
        gpio_write(GPIOG_BASE, 14, 0);
        os_delay(700);
    }
}

int main(void) {
    system_init();
    uart_init(USART1_BASE);
    rcc_enable_gpio(GPIOG_EN);
    os_heap_init();
    os_task_create(task1, 2, 128);
    os_task_create(task2, 2, 128);

    os_start();

    return 0;
}
