#include "heap.h"
#include "mutex.h"
#include "mpu.h"

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((uint32_t)(a) - 1))

extern uint8_t _sheap;
extern uint8_t _eheap;
static heap_block_t *heap_head = 0;
os_mutex_t mutex_heap;

void os_heap_init() {
    os_mutex_init(&mutex_heap);
    heap_head = (heap_block_t *)&_sheap;
    heap_head->is_free = 1;
    heap_head->size = (uint8_t *)&_eheap - (uint8_t *)&_sheap - sizeof(heap_block_t);
    heap_head->next = 0;
}

void *os_malloc(uint32_t size) {
    os_mutex_take(&mutex_heap);
    heap_block_t *current = heap_head;
    uint32_t aligned_size;
    if(mpu_status == 0) {
        aligned_size = ALIGN(size, 8);
    }
    else {
        aligned_size = ALIGN(size, 32);
    }
    while(current != 0) {
        if(current->is_free == 1 && current->size >= aligned_size) {
            if(current->size > aligned_size + sizeof(heap_block_t)) {
                heap_block_t *new = current;
                new = (heap_block_t *)((uint8_t *)current + sizeof(heap_block_t) + aligned_size);
                new->is_free = 1;
                new->size = current->size - aligned_size - sizeof(heap_block_t);
                new->next = current->next;
                current->is_free = 0;
                current->size = aligned_size;
                current->next = new;
            }
            else {
                current->is_free = 0;
            }
            os_mutex_give(&mutex_heap);
            return (void *)((uint8_t *)current + sizeof(heap_block_t));
        }
        current = current->next;
    }
    os_mutex_give(&mutex_heap);
    return 0;
}

void os_mfree(void *mem) {
    if(mem == 0) {
        return;
    }
    os_mutex_take(&mutex_heap);
    heap_block_t *block = (heap_block_t *)((uint8_t *)mem - sizeof(heap_block_t));
    block->is_free = 1;
    heap_block_t *current = heap_head;
    while(current->next != 0) {
        if(current->is_free == 1 && current->next->is_free == 1) {
            current->size = current->size + sizeof(heap_block_t) + current->next->size;
            current->next = current->next->next;
        }
        else {
            current = current->next;
        }
    }
    os_mutex_give(&mutex_heap);
}
