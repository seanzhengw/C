#include "cbuf.h"
#include <stdlib.h>
#include <string.h>

static inline void *cbuf_raw(struct cbuf *ptrcbuffer)
{
    return (ptrcbuffer)->buf;
}

static inline unsigned int cbuf_rawsize(struct cbuf *ptrcbuffer)
{
    return cbuf_size(ptrcbuffer) * cbuf_element_size(ptrcbuffer);
}

static inline void *cbuf_rawput_pos(struct cbuf *ptrcbuffer)
{
    return (cbuf_raw(ptrcbuffer) + cbuf_put_pos(ptrcbuffer) * cbuf_element_size(ptrcbuffer));
}

static inline void *cbuf_rawget_pos(struct cbuf *ptrcbuffer)
{
    return (cbuf_raw(ptrcbuffer) + cbuf_get_pos(ptrcbuffer) * cbuf_element_size(ptrcbuffer));
}

static inline unsigned int cbuf_rawavail(struct cbuf *ptrcbuffer)
{
    return (cbuf_avail(ptrcbuffer) * cbuf_element_size(ptrcbuffer));
}

static inline unsigned int cbuf_rawlen(struct cbuf *ptrcbuffer)
{
    return (cbuf_len(ptrcbuffer) * cbuf_element_size(ptrcbuffer));
}

int cbuf_init(struct cbuf *ptrcbuffer, void *buffer, unsigned int size, size_t esize)
{
    size /= esize;
    if (size == 0)
    {
        ptrcbuffer->ecount = 0;
        return -1;
    }

    ptrcbuffer->wpos = 0;
    ptrcbuffer->rpos = 0;
    ptrcbuffer->esize = esize;
    ptrcbuffer->ecount = size;
    ptrcbuffer->buf = buffer;

    return 0;
}

int cbuf_alloc(struct cbuf *ptrcbuffer, unsigned int ecount, size_t esize)
{
    if (ecount == 0)
    {
        ptrcbuffer->ecount = 0;
        return -1;
    }

    ptrcbuffer->buf = malloc(ecount * esize);
    if (ptrcbuffer->buf == NULL)
    {
        return -1;
    }

    ptrcbuffer->wpos = 0;
    ptrcbuffer->rpos = 0;
    ptrcbuffer->esize = esize;
    ptrcbuffer->ecount = ecount;

    return 0;
}

int cbuf_free(struct cbuf *ptrcbuffer)
{
    free(ptrcbuffer->buf);
    ptrcbuffer->wpos = 0;
    ptrcbuffer->rpos = 0;
    ptrcbuffer->esize = 0;
    ptrcbuffer->ecount = 0;
}

int cbuf_compact(struct cbuf *ptrcbuffer)
{
    if (cbuf_get_pos(ptrcbuffer) == 0)
    {
        return -1;
    }
    unsigned int size = cbuf_rawavail(ptrcbuffer);
    memmove(cbuf_raw(ptrcbuffer), cbuf_rawget_pos(ptrcbuffer), size);
    cbuf_get_pos(ptrcbuffer) = 0;
    cbuf_put_pos(ptrcbuffer) = size;
    return 0;
}

unsigned int cbuf_put(struct cbuf *ptrcbuffer, const void *buf)
{
    if (cbuf_avail(ptrcbuffer))
    {
        memcpy(cbuf_rawput_pos(ptrcbuffer), buf, cbuf_element_size(ptrcbuffer));
        cbuf_put_pos(ptrcbuffer)++;
        return 1;
    }
    return 0;
}

unsigned int cbuf_write(struct cbuf *ptrcbuffer, const void *buf, unsigned int ecount)
{
    if (cbuf_avail(ptrcbuffer) < ecount)
    {
        ecount = cbuf_avail(ptrcbuffer);
    }
    memcpy(cbuf_rawput_pos(ptrcbuffer), buf, cbuf_element_size(ptrcbuffer) * ecount);
    cbuf_put_pos(ptrcbuffer) += ecount;
    return ecount;
}

unsigned int cbuf_get(struct cbuf *ptrcbuffer, void *buf)
{
    if (cbuf_len(ptrcbuffer))
    {
        memcpy(buf, cbuf_rawget_pos(ptrcbuffer), cbuf_element_size(ptrcbuffer));
        memset(cbuf_rawget_pos(ptrcbuffer), 0, cbuf_element_size(ptrcbuffer));
        cbuf_get_pos(ptrcbuffer)++;
        return 1;
    }
    return 0;
}

unsigned int cbuf_read(struct cbuf *ptrcbuffer, void *buf, unsigned int ecount)
{
    if (cbuf_len(ptrcbuffer) < ecount)
    {
        ecount = cbuf_len(ptrcbuffer);
    }
    memcpy(buf, cbuf_rawget_pos(ptrcbuffer), cbuf_element_size(ptrcbuffer) * ecount);
    memset(cbuf_rawget_pos(ptrcbuffer), 0, cbuf_element_size(ptrcbuffer) * ecount);
    cbuf_get_pos(ptrcbuffer) += ecount;
    return ecount;
}

unsigned int cbuf_peek(struct cbuf *ptrcbuffer, void *buf)
{
    if (cbuf_len(ptrcbuffer))
    {
        memcpy(buf, cbuf_rawget_pos(ptrcbuffer), cbuf_element_size(ptrcbuffer));
    }
}
