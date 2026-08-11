#include "system_init.h"
#include "rcc.h"
#include "gpio.h"
#include "task.h"
#include "heap.h"

#define LED_PORT GPIOG_BASE
#define LED_PIN 13
#define BUTTON_PORT GPIOA_BASE
#define BUTTON_PIN 0

void button_task(void) {
    gpio_set_mode(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_mode(BUTTON_PORT, BUTTON_PIN, GPIO_MODE_INPUT);
    while(1) {
        if(gpio_read(BUTTON_PORT, BUTTON_PIN)) {
            gpio_write(LED_PORT, LED_PIN, 1);
        }
        else {
            gpio_write(LED_PORT, LED_PIN, 0);
        }
    }
}

int main(void) {
    system_init();
    rcc_enable_gpio(GPIOG_EN);
    rcc_enable_gpio(GPIOA_EN);
    os_heap_init();
    os_task_create(button_task, 2, 128);

    os_start();

    return 0;
}
