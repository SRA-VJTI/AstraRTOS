#ifndef H_TASK
#define H_TASK
#include <stdint.h>
#define OS_MAX_TASKS_NUM 20

typedef enum {
    TASK_READY,
    TASK_BLOCKED,
    TASK_MUTEX_WAITING,
    TASK_SEM_WAITING,
    TASK_QUEUE_SEND_WAITING,
    TASK_QUEUE_RECV_WAITING
} os_task_state;

typedef struct {
    uint32_t *stack_ptr;
    uint32_t *stack_base;
    uint32_t stack_size;
    int task_num;
    os_task_state state;
    uint32_t delay_ticks;
    void *waiting_for_resource;
    uint32_t priority;
    uint32_t base_priority;
    uint32_t run_count;
    uint32_t stack_high_water;
} os_tcb_t;

extern os_tcb_t *os_tasks[];
extern os_tcb_t *os_current_task_ptr;
void os_delay(uint32_t t);
void os_decrement_blocked_tasks(void);
extern void os_schedule_next_task(void);
os_tcb_t *os_task_create(void (*task_fucntion)(void), uint32_t priority, uint32_t stack_size);
void os_start(void);
#endif
