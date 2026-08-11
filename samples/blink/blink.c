#include "system_init.h"
#include "rcc.h"
#include "gpio.h"
#include "task.h"
#include "heap.h"

void blink_task(void) {
    gpio_set_mode(GPIOG_BASE, 13, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_write(GPIOG_BASE, 13, 1);
        os_delay(500);
        gpio_write(GPIOG_BASE, 13, 0);
        os_delay(500);
    }
}

int main(void) {
    system_init();
    rcc_enable_gpio(GPIOG_EN);
    os_heap_init();
    os_task_create(blink_task, 2, 128);

    os_start();

    return 0;
}
