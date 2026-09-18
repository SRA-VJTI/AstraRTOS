#include "stack_pool.h"

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((uint32_t)(a) - 1))

extern uint8_t _sstacks;
extern uint8_t _estacks;

static uint8_t *stack_pool_ptr;

void os_stack_pool_init() {
    stack_pool_ptr = &_sstacks;
}

uint8_t *os_stack_pool_alloc(uint32_t size, uint32_t alignment) {
    uint32_t addr = (uint32_t)stack_pool_ptr;
    addr = ALIGN(addr, alignment);
    uint8_t *result = (uint8_t *)addr;

    if(result + size > &_estacks) {
        return 0;
    }

    stack_pool_ptr = (result + size);
    return result; /* return to start of allocated stack */
}
