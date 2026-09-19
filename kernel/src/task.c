#include "task.h"
#include "heap.h"
#include "stack_pool.h"
#include "../port/system_init.h"
#define SHPR3 (*(volatile uint8_t *)0xE000ED22) /*sets PendSV priority*/
#define ICSR (*((volatile uint32_t *)0xE000ED04))
os_tcb_t *os_tasks[OS_MAX_TASKS_NUM];
os_tcb_t *os_current_task_ptr = 0;
static uint32_t os_task_count = 0;

os_tcb_t *os_task_create(void (*task_function)(void), uint32_t priority, uint32_t stack_size) {
    if(os_task_count >= OS_MAX_TASKS_NUM) {
        return -1;
    }
    os_tcb_t *task = (os_tcb_t *)os_malloc(sizeof(os_tcb_t));
    if(task == 0) {
        return -1;
    }
    uint32_t *task_stack = (uint32_t *)os_stack_pool_alloc((sizeof(uint32_t) * stack_size), 32);
    if(task_stack == 0) {
        os_mfree(task);
        return -1;
    }
    uint32_t *task_stack_ptr = &task_stack[stack_size - 1];
    *task_stack_ptr = 0x01000000;
    task_stack_ptr -= 1;
    *task_stack_ptr = (uint32_t)task_function;
    task_stack_ptr -= 1;
    *task_stack_ptr = 0xFFFFFFFD;
    task_stack_ptr -= 13;
    task->stack_ptr = task_stack_ptr;
    task->stack_base = task_stack;
    task->stack_size = stack_size;
    task->task_num = os_task_count;
    task->state = TASK_READY;
    task->delay_ticks = 0;
    task->priority = priority;
    task->base_priority = priority;
    task->run_count = 0;
    task->stack_high_water = 0;
    if(os_task_count == 0) {
        os_current_task_ptr = task;
    }
    os_tasks[os_task_count] = task;
    os_task_count += 1;
    return task;
}

void os_delay(uint32_t ticks) {
    __asm volatile("cpsid i");
    os_current_task_ptr->delay_ticks = ticks;
    os_current_task_ptr->state = TASK_BLOCKED;
    __asm volatile("cpsie i");
    ICSR |= (1 << 28);
}

void os_decrement_blocked_tasks(void) {
    for(uint32_t i = 0; i < os_task_count; i++) {
        if(os_tasks[i]->state == TASK_BLOCKED && os_tasks[i]->delay_ticks > 0) {
            os_tasks[i]->delay_ticks -= 1;
            if(os_tasks[i]->delay_ticks == 0) {
                os_tasks[i]->state = TASK_READY;
            }
        }
    }
}

void os_schedule_next_task(void) {
    uint32_t best_priority = -1;
    int chosen_index = -1;
    if(os_task_count > 0) {
        int next_task = (os_current_task_ptr->task_num + 1) % os_task_count;
        for(uint32_t i = 0; i < os_task_count; i++) {
            if(os_tasks[next_task]->state == TASK_READY) {
                if(chosen_index == -1 || (os_tasks[next_task]->priority > best_priority)) {
                    best_priority = os_tasks[next_task]->priority;
                    chosen_index = next_task;
                }
            }
            next_task = (next_task + 1) % os_task_count;
        }
        if(chosen_index != -1) {
            os_tasks[chosen_index]->run_count += 1;
            uint32_t used = (uint32_t)((uint8_t *)&os_tasks[chosen_index]
                                           ->stack_base[os_tasks[chosen_index]->stack_size - 1] -
                                       (uint8_t *)os_tasks[chosen_index]->stack_ptr);
            if(used > os_tasks[chosen_index]->stack_high_water) {
                os_tasks[chosen_index]->stack_high_water = used;
            }
            os_current_task_ptr = os_tasks[chosen_index];
        }
    }
}

static void os_idle_task(void) {
    while(1) {
        __asm volatile("wfi"); // CPU sleep
    }
}

void os_start(void) {
    os_task_create(os_idle_task, 0, 64); // default to sleep
    SHPR3 = 255;                         /*sets PendSV to lowest priority*/
    systick_init();                      // enables systick
    __asm volatile("svc 0");             /*triggers SVC*/
}

uint32_t os_get_task_count() {
    return os_task_count;
}
