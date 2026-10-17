#include "tlsf.h"

#define ALIGN(x) ((x + TLSF_ALIGN - 1) & ~(TLSF_ALIGN - 1))

static uint32_t find_fls(uint32_t fls_bitmap) {
    uint32_t position = 0;

    if(fls_bitmap == 0) {
        return 0; /* none free */
    }

    while(fls_bitmap > 1) {
        fls_bitmap = fls_bitmap >> 1;
        position++;
    }

    return position;
}

static void mapping_insert(uint32_t size, uint32_t *fli, uint32_t *sli) {
    if(size < (1 << TLSF_OFFSET)) {
        *fli = 0;
        *sli = size >> (TLSF_OFFSET - TLSF_SLI_LOG2);
    }
    else {
        uint32_t f = find_fls(size);
        *fli = f - TLSF_OFFSET;
        *sli = (size >> (f - TLSF_SLI_LOG2) & (TLSF_SLI_COUNT - 1));
    }
}

static void mapping_search(uint32_t size, uint32_t *fli, uint32_t *sli) {
    if(size >= (1 << TLSF_OFFSET)) {
        uint32_t f = find_fls(size);
        uint32_t round = ((1 << (f - TLSF_SLI_LOG2)) - 1);
        size += round; /* round to the next nearest block size */
    }

    mapping_insert(size, fli, sli);
}

/* initialise tlsf with heap as first block */
void tlsf_init(tlsf_t *t) {
    t->fli_bitmap = 0;

    for(uint32_t i = 0; i < TLSF_FLI_COUNT; i++) {
        t->sli_bitmap[i] = 0;

        for(uint32_t j = 0; j < TLSF_SLI_COUNT; j++) {
            t->blocks[i][j] = 0;
        }
    }

    t->first_block = (tlsf_block_t *)&_sheap;
    t->pool_size = (uint8_t *)&_eheap - (uint8_t *)&_sheap;

    tlsf_block_t *block = t->first_block;
    uint32_t size_block = t->pool_size - TLSF_HEADER_SIZE;

    block->prev_physical = 0;
    block->next_free = 0;
    block->prev_free = 0;
    block->size_flag = size_block | TLSF_FREE_BIT;

    uint32_t fli;
    uint32_t sli;

    mapping_insert(size_block, &fli, &sli);
    t->fli_bitmap |= 1 << fli;
    t->sli_bitmap[fli] |= 1 << sli;
    t->blocks[fli][sli] = block;
}

static void insert_free_block(tlsf_t *t, tlsf_block_t *block) {
    uint32_t fli;
    uint32_t sli;
    uint32_t block_size = block->size_flag & TLSF_SIZE_MASK;

    mapping_insert(block_size, &fli, &sli);

    block->next_free = t->blocks[fli][sli];
    block->prev_free = 0;
    if(t->blocks[fli][sli] != 0) {
        t->blocks[fli][sli]->prev_free = block;
    }
    t->blocks[fli][sli] = block;
    t->fli_bitmap |= 1 << fli;
    t->sli_bitmap[fli] |= 1 << sli;
}

static void remove_free_block(tlsf_t *t, tlsf_block_t *block) {
    uint32_t fli;
    uint32_t sli;
    uint32_t block_size = block->size_flag & TLSF_SIZE_MASK;

    mapping_insert(block_size, &fli, &sli);

    if(block->prev_free != 0) {
        block->prev_free->next_free = block->next_free;
    }
    else {
        t->blocks[fli][sli] = block->next_free;
    }

    if(block->next_free != 0) {
        block->next_free->prev_free = block->prev_free;
    }

    if(t->blocks[fli][sli] == 0) {
        t->sli_bitmap[fli] &= ~(1 << sli);

        if(t->sli_bitmap[fli] == 0) {
            t->fli_bitmap &= ~(1 << fli);
        }
    }

    block->next_free = 0;
    block->prev_free = 0;
}

static tlsf_block_t *find_free_block(tlsf_t *t, uint32_t fli, uint32_t sli) {
    for(uint32_t i = fli; i < TLSF_FLI_COUNT; i++) {
        if((t->fli_bitmap >> i) & 0x1) {
            if(t->sli_bitmap[i] != 0) {
                uint32_t start_sli;

                if(i == fli) {
                    start_sli = sli;
                }
                else {
                    start_sli = 0;
                }

                for(uint32_t j = start_sli; j < TLSF_SLI_COUNT; j++) {
                    if((t->sli_bitmap[i] >> j) & 0x1) {
                        return t->blocks[i][j];
                    }
                }
            }
        }
    }
    return 0; /* heap full */
}

void *tlsf_malloc(tlsf_t *t, uint32_t size) {
    if(t == 0 || size == 0) {
        return 0;
    }

    uint32_t size_aligned = ALIGN(size);

    if(size_aligned < TLSF_MIN_BLOCK_SIZE) {
        size_aligned = TLSF_MIN_BLOCK_SIZE;
    }

    if(size_aligned > t->pool_size - TLSF_HEADER_SIZE) {
        return 0;
    }

    uint32_t fli, sli;
    mapping_search(size_aligned, &fli, &sli);

    tlsf_block_t *block = find_free_block(t, fli, sli);
    if(block == 0) {
        return 0;
    }

    uint32_t size_block = block->size_flag & TLSF_SIZE_MASK;
    remove_free_block(t, block);

    uint32_t split = size_block - size_aligned;
    if(split >= TLSF_MIN_BLOCK_SIZE + TLSF_HEADER_SIZE) {
        tlsf_block_t *remaining =
            (tlsf_block_t *)((uint8_t *)block + TLSF_HEADER_SIZE + size_aligned);
        uint32_t remaining_size = split - TLSF_HEADER_SIZE;
        remaining->prev_physical = block;
        remaining->size_flag = remaining_size | TLSF_FREE_BIT;
        remaining->next_free = 0;
        remaining->prev_free = 0;

        uint32_t prev_flag = block->size_flag & TLSF_PREV_FREE_BIT;
        block->size_flag = size_aligned | prev_flag; /* marks this block as not free too */
        uint8_t *next_address = ((uint8_t *)remaining + TLSF_HEADER_SIZE + remaining_size);
        uint8_t *end_of_pool = ((uint8_t *)t->first_block + t->pool_size);

        if(next_address < end_of_pool) {
            tlsf_block_t *next = (tlsf_block_t *)next_address;
            next->prev_physical = remaining;
            next->size_flag |= TLSF_PREV_FREE_BIT;
        }
        insert_free_block(t, remaining);
    }
    else {
        block->size_flag &= ~TLSF_FREE_BIT;

        uint8_t *next_address = (uint8_t *)block + TLSF_HEADER_SIZE + size_block;
        uint8_t *end_of_pool = (uint8_t *)t->first_block + t->pool_size;

        if(next_address < end_of_pool) {
            tlsf_block_t *next = (tlsf_block_t *)next_address;
            next->size_flag &= ~TLSF_PREV_FREE_BIT;
        }
    }
    return (uint8_t *)block + TLSF_HEADER_SIZE;
}

void tlsf_free(tlsf_t *t, void *ptr) {
    if(t == 0 || ptr == 0) {
        return;
    }

    tlsf_block_t *block = (tlsf_block_t *)((uint8_t *)ptr - TLSF_HEADER_SIZE);
    block->size_flag |= TLSF_FREE_BIT;
    tlsf_block_t *prev = block->prev_physical;

    /* merge if prev one is free */
    if(prev != 0 && (prev->size_flag & TLSF_FREE_BIT)) {
        remove_free_block(t, prev);
        uint32_t prev_size = prev->size_flag & TLSF_SIZE_MASK;
        uint32_t block_size = block->size_flag & TLSF_SIZE_MASK;
        uint32_t merged_size = prev_size + block_size + TLSF_HEADER_SIZE;
        uint32_t prev_flag = prev->size_flag & TLSF_PREV_FREE_BIT;
        prev->size_flag = merged_size | prev_flag | TLSF_FREE_BIT;
        block = prev;
    }

    uint32_t block_size = block->size_flag & TLSF_SIZE_MASK;
    uint8_t *next_address = (uint8_t *)block + TLSF_HEADER_SIZE + block_size;
    uint8_t *pool_end = (uint8_t *)t->first_block + t->pool_size;

    /* merge next if free */
    if(next_address < pool_end) {
        tlsf_block_t *next = (tlsf_block_t *)next_address;

        if(next->size_flag & TLSF_FREE_BIT) {
            remove_free_block(t, next);
            uint32_t next_size = next->size_flag & TLSF_SIZE_MASK;
            uint32_t merged_size = next_size + block_size + TLSF_HEADER_SIZE;
            uint32_t block_flag = block->size_flag & TLSF_PREV_FREE_BIT;
            block->size_flag = merged_size | block_flag | TLSF_FREE_BIT;
        }
    }
    uint32_t final_size = block->size_flag & TLSF_SIZE_MASK;
    uint8_t *after_address = (uint8_t *)block + TLSF_HEADER_SIZE + final_size;

    if(after_address < pool_end) {
        tlsf_block_t *after = (tlsf_block_t *)after_address;
        after->prev_physical = block;
        after->size_flag |= TLSF_PREV_FREE_BIT;
    }
    insert_free_block(t, block);
}
