#ifndef C_LIB_CBUF_H_
#define C_LIB_CBUF_H_

#include <stddef.h>

struct cbuf
{
    unsigned int wpos;   /* next write position */
    unsigned int rpos;   /* next read position */
    unsigned int ecount; /* elements count */
    unsigned int esize;  /* sizeof(Element) */
    void *buf;           /* elements data buffer */
};

#define cbuf_put_pos(ptrcbuffer) ((ptrcbuffer)->wpos)
#define cbuf_get_pos(ptrcbuffer) ((ptrcbuffer)->rpos)
#define cbuf_size(ptrcbuffer) ((ptrcbuffer)->ecount)
#define cbuf_element_size(ptrcbuffer) ((ptrcbuffer)->esize)

extern int cbuf_init(struct cbuf *ptrcbuffer, void *buffer, unsigned int size, size_t esize);
extern int cbuf_alloc(struct cbuf *ptrcbuffer, unsigned int ecount, size_t esize);
extern int cbuf_free(struct cbuf *ptrcbuffer);
extern int cbuf_compact(struct cbuf *ptrcbuffer);
extern unsigned int cbuf_put(struct cbuf *ptrcbuffer, const void *buf);
extern unsigned int cbuf_write(struct cbuf *ptrcbuffer, const void *buf, unsigned int ecount);
extern unsigned int cbuf_get(struct cbuf *ptrcbuffer, void *buf);
extern unsigned int cbuf_read(struct cbuf *ptrcbuffer, void *buf, unsigned int ecount);
extern unsigned int cbuf_peek(struct cbuf *ptrcbuffer, void *buf);

#define cbuf_avail(ptrcbuffer) (cbuf_size(ptrcbuffer) - cbuf_put_pos(ptrcbuffer))

#define cbuf_len(ptrcbuffer) (cbuf_put_pos(ptrcbuffer) - cbuf_get_pos(ptrcbuffer))

#define cbuf_reset(ptrcbuffer) ({ \
    cbuf_put_pos(ptrcbuffer) = 0; \
    cbuf_get_pos(ptrcbuffer) = 0; \
})

#define cbuf_is_empty(ptrcbuffer) (cbuf_put_pos(ptrcbuffer) == cbuf_get_pos(ptrcbuffer))

#endif
