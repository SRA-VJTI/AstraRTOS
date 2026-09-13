#include "system_init.h"
#include "task.h"
#include "heap.h"
#include "timer.h"

static os_timer_t timer_a;
static os_timer_t timer_b;

static volatile uint32_t timer_a_fired = 0;
static volatile uint32_t timer_b_fired = 0;

static void on_timer_a(void) {
    timer_a_fired += 1;
}

static void on_timer_b(void) {
    timer_b_fired += 1;
}

static void task_timer_demo(void) {
    os_timer_init(&timer_a, 500, 0, on_timer_a);
    os_timer_init(&timer_b, 1000, 1, on_timer_b);
    os_timer_start(&timer_a);
    os_timer_start(&timer_b);
    while(1) {
        if(timer_a_fired > 0) {
            timer_a_fired = 0;
            os_timer_start(&timer_a);
        }
        os_delay(10);
    }
}

int main(void) {
    system_init();
    os_heap_init();
    os_task_create(task_timer_demo, 2, 256);
    os_start();
    return 0;
}
