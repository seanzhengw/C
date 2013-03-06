#ifndef C_LIB_MEMPOOL_H_
#define C_LIB_MEMPOOL_H_

#include <stdint.h>
#include <stddef.h>

struct mem_block_info
{
    uint32_t used : 1;  /* Is the block in use. */
    uint32_t size : 31; /* block size (bytes), include block header (this mem_block_info) */
};

struct mempool
{
    uint32_t size;                      /* memory pool size */
    struct mem_block_info *first_block; /* first memory block */
};

extern int mempool_init(struct mempool *pMempool, void *buffer, size_t size);
extern size_t mempool_avail(struct mempool *pMempool);
extern void *mempool_alloc(struct mempool *pMempool, size_t nbytes);
extern int mempool_free(struct mempool *pMempool, void *p);
extern int mempool_has(struct mempool *pMempool, void *p);
extern size_t mempool_max_continuous(struct mempool *pMempool);

#endif
