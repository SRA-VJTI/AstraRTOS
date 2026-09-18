#ifndef STACK_POOL_H
#define STACK_POOL_H

#include <stdint.h>

void os_stack_pool_init();
uint8_t *os_stack_pool_alloc(uint32_t size, uint32_t alignment);

#endif
