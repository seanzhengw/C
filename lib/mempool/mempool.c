#include "mempool.h"

static inline struct mem_block_info *mempool_end(struct mempool *pMempool)
{
    return (struct mem_block_info *)(((char *)pMempool->first_block) + pMempool->size);
}

static inline struct mem_block_info *mem_block_next(struct mem_block_info *block)
{
    return (struct mem_block_info *)(((uint8_t *)block) + block->size);
}

int mempool_init(struct mempool *pMempool, void *buffer, size_t size)
{
    if (size < 4)
    {
        return -1;
    }
    pMempool->first_block = (struct mem_block_info *)buffer;
    pMempool->size = size;
    pMempool->first_block->used = 0;
    pMempool->first_block->size = size;
    return 0;
}

size_t mempool_avail(struct mempool *pMempool)
{
    size_t sum = 0;
    const struct mem_block_info *poolend = mempool_end(pMempool);
    for (struct mem_block_info *p = pMempool->first_block; p < poolend; p = mem_block_next(p))
    {
        if (p->used == 0)
        {
            sum += p->size - sizeof(struct mem_block_info);
        }
    }
    return sum;
}

void *mempool_alloc(struct mempool *pMempool, size_t nbytes)
{
    /* 4 byte padding, like c structure padding */
    nbytes += (nbytes & 3) ? 4 - (nbytes & 3) : 0;
    /* block size include the header (mem_block_info) */
    size_t blocksize = nbytes + sizeof(struct mem_block_info);

    /* start looking for available block */
    struct mem_block_info *p;
    /* the end of memory pool, this position is out of buffer */
    const struct mem_block_info *poolend = (struct mem_block_info *)(((char *)pMempool->first_block) + pMempool->size);
    for (p = pMempool->first_block; p < poolend; p = (struct mem_block_info *)(((char *)p) + p->size))
    {
        /* check block is not allocated and size enough */
        if (p->used == 1 || p->size < blocksize)
        {
            continue;
        }
        /* found available block */
        p->used = 1;

        /*
         * check the block has more space than blocksize
         * if p->size > blocksize, insert new block between this block and the next block
         * otherwise, just use this block.
         */
        if (p->size > blocksize)
        {
            struct mem_block_info *nextblock = (struct mem_block_info *)(((char *)p) + blocksize);
            nextblock->used = 0;
            nextblock->size = p->size - blocksize;
            p->size = blocksize;
        }

        /* return addr */
        return p + 1;
    }

    /* there is no space for nbytes */
    return NULL;
}

int mempool_free(struct mempool *pMempool, void *p)
{
    /* the end of memory pool, this position is out of buffer */
    const struct mem_block_info *poolend = (struct mem_block_info *)(((char *)pMempool->first_block) + pMempool->size);

    struct mem_block_info *block = ((struct mem_block_info *)p) - 1;
    /* check block is inside this mempool */
    if (block < pMempool->first_block || block > poolend)
    {
        return -1;
    }

    /* find previous block if block is not first block of this mempool*/
    if (block != pMempool->first_block)
    {
        struct mem_block_info *prev = pMempool->first_block;
        for (; prev->size && prev < block;)
        {
            struct mem_block_info *prevnext = mem_block_next(prev);
            /* check the next block of prev is this block */
            if (prevnext == block)
            {
                break;
            }
            prev = prevnext;
        }
        /* return -1 if the next block of prev is this NOT this block. */
        if (block != mem_block_next(prev))
        {
            return -1;
        }
        /* combine prev with this block if prev is unused. */
        if (prev->used == 0)
        {
            prev->size += block->size;
            block = prev;
        }
    }

    /* 
     * if this block is at the end of pool, no need to combine next block,
     * and if next block is unused, combine with this block.
     */
    struct mem_block_info *next = mem_block_next(block);
    if (next != poolend)
    {
        if (next->used == 0)
        {
            block->size += next->size;
        }
    }

    /* mark this block unused */
    block->used = 0;

    return 0;
}

int mempool_has(struct mempool *pMempool, void *p)
{
    /* the end of memory pool, this position is out of buffer */
    const struct mem_block_info *poolend = (struct mem_block_info *)(((char *)pMempool->first_block) + pMempool->size);

    struct mem_block_info *block = ((struct mem_block_info *)p) - 1;
    /* check block is inside this mempool */
    if (block < pMempool->first_block || block > poolend)
    {
        return 0;
    }

    /* find block */
    struct mem_block_info *found = pMempool->first_block;
    for (; found->size && found < block;)
    {
        found = mem_block_next(found);
    }

    if (found != block)
    {
        /* block is not recognized */
        return 0;
    }

    return block->used;
}

extern size_t mempool_max_continuous(struct mempool *pMempool)
{
    size_t maxsize = 0;
    const struct mem_block_info *poolend = mempool_end(pMempool);
    for (struct mem_block_info *p = pMempool->first_block; p < poolend; p = mem_block_next(p))
    {
        if (p->used == 0)
        {
            size_t usable = p->size - sizeof(struct mem_block_info);
            if (usable > maxsize)
            {
                maxsize = usable;
            }
        }
    }
    return maxsize;
}
