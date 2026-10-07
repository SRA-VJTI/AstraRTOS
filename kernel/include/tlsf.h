#ifndef TLSF_H
#define TLSF_H

#include <stdint.h>

#define TLSF_FLI_COUNT 16
#define TLSF_SLI_LOG2 3
#define TLSF_SLI_COUNT (1 << TLSF_SLI_LOG2)
#define TLSF_OFFSET 4
#define TLSF_ALIGN 4
#define TLSF_MIN_BLOCK_SIZE 16

#define TLSF_FREE_BIT 0x1
#define TLSF_PREV_FREE_BIT 0x2
#define TLSF_SIZE_MASK ~(0x3)

extern uint8_t _eheap;
extern uint8_t _sheap;

typedef struct tlsf_block {
    struct tlsf_block *prev_physical;
    uint32_t size_flag;
    struct tlsf_block *next_free;
    struct tlsf_block *prev_free;
} tlsf_block_t;

#define TLSF_HEADER_SIZE sizeof(tlsf_block_t)

typedef struct tlsf_t {
    uint32_t fli_bitmap;
    uint32_t sli_bitmap[TLSF_FLI_COUNT];
    tlsf_block_t *blocks[TLSF_FLI_COUNT][TLSF_SLI_COUNT];
    tlsf_block_t *first_block;
    uint32_t pool_size;
} tlsf_t;

void tlsf_init(tlsf_t *t);
void *tlsf_malloc(tlsf_t *t, uint32_t size);
void tlsf_free(tlsf_t *t, void *ptr);

#endif
