#include "tlsf.h"

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
