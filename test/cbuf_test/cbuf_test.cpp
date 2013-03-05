#include <stdio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C"
{
#include <cbuf/cbuf.h>
}

const static unsigned int bsize = 128;
const size_t esize = sizeof(int);
const size_t ecount = bsize / sizeof(int);

class cbufTest : public ::testing::Test
{
protected:
    cbufTest() {}
    virtual ~cbufTest() {}
    virtual void SetUp() override
    {
        cbuf_init(&mycbuf, buffer, bsize, esize);
    }
    virtual void TearDown() override
    {
        cbuf_reset(&mycbuf);
    }

    char buffer[bsize];
    struct cbuf mycbuf;

    void *cbuf_raw(struct cbuf *ptrcbuffer)
    {
        return (ptrcbuffer)->buf;
    }

    unsigned int cbuf_rawsize(struct cbuf *ptrcbuffer)
    {
        return cbuf_size(ptrcbuffer) * cbuf_element_size(ptrcbuffer);
    }

    unsigned int cbuf_rawavail(struct cbuf *ptrcbuffer)
    {
        return (cbuf_avail(ptrcbuffer) * cbuf_element_size(ptrcbuffer));
    }

    unsigned int cbuf_rawlen(struct cbuf *ptrcbuffer)
    {
        return (cbuf_len(ptrcbuffer) * cbuf_element_size(ptrcbuffer));
    }
};

TEST_F(cbufTest, Init)
{
    ASSERT_TRUE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount);
    ASSERT_EQ(cbuf_rawavail(&mycbuf), bsize);
    ASSERT_EQ(cbuf_len(&mycbuf), 0);
    ASSERT_EQ(cbuf_rawlen(&mycbuf), 0);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);
}

TEST_F(cbufTest, Puts)
{
    int e = 100;
    int ret = 0;

    ret = cbuf_put(&mycbuf, &e);

    ASSERT_EQ(ret, 1);
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_rawsize(&mycbuf), bsize);
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - 1);
    ASSERT_EQ(cbuf_len(&mycbuf), 1);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);

    e = 200;
    ret = cbuf_put(&mycbuf, &e);

    ASSERT_EQ(ret, 1);
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_rawsize(&mycbuf), bsize);
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - 2);
    ASSERT_EQ(cbuf_len(&mycbuf), 2);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);
}

TEST_F(cbufTest, Reset)
{
    ASSERT_TRUE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_rawsize(&mycbuf), bsize);
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount);
    ASSERT_EQ(cbuf_len(&mycbuf), 0);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);
}

TEST_F(cbufTest, Writes)
{
    int e[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int writes = 3;
    int writes2 = 5;
    int writes_all = writes + writes2;
    int ret = 0;

    ret = cbuf_write(&mycbuf, &e, writes);

    ASSERT_EQ(ret, writes);
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_rawsize(&mycbuf), bsize);
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - writes);
    ASSERT_EQ(cbuf_len(&mycbuf), writes);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);

    ret = cbuf_write(&mycbuf, (&e) + writes, writes2);

    ASSERT_EQ(ret, writes2);
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_rawsize(&mycbuf), bsize);
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - writes_all);
    ASSERT_EQ(cbuf_len(&mycbuf), writes_all);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);
}

TEST_F(cbufTest, Gets)
{
    int writes = 2;
    int reads = 0;
    for (size_t e = 0; e < writes; e++)
    {
        cbuf_put(&mycbuf, &e);
    }

    int e;
    int ret = 0;
    ret = cbuf_get(&mycbuf, &e);
    reads++;

    ASSERT_EQ(ret, 1);
    ASSERT_EQ(e, 0);
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - writes);
    ASSERT_EQ(cbuf_len(&mycbuf), writes - reads);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);

    ret = cbuf_get(&mycbuf, &e);
    reads++;

    ASSERT_EQ(ret, 1);
    ASSERT_EQ(e, 1);
    ASSERT_TRUE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - writes);
    ASSERT_EQ(cbuf_len(&mycbuf), writes - reads);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);
}

TEST_F(cbufTest, Reads)
{
    int writes = 8;
    int reads = 0;
    for (size_t e = 0; e < writes; e++)
    {
        cbuf_put(&mycbuf, &e);
    }

    int e[8];
    memset(e, 0, 8 * sizeof(*e));
    int ret = 0;
    ret = cbuf_read(&mycbuf, e, 5);
    reads = 5;

    ASSERT_EQ(ret, reads);
    ASSERT_THAT(e, ::testing::ElementsAre(0, 1, 2, 3, 4, 0, 0, 0));
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - writes);
    ASSERT_EQ(cbuf_len(&mycbuf), writes - reads);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);

    ret = cbuf_read(&mycbuf, e + reads, 3);
    reads += 3;

    ASSERT_EQ(ret, 3);
    ASSERT_THAT(e, ::testing::ElementsAre(0, 1, 2, 3, 4, 5, 6, 7));
    ASSERT_TRUE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_avail(&mycbuf), ecount - writes);
    ASSERT_EQ(cbuf_len(&mycbuf), writes - reads);
    ASSERT_EQ(cbuf_raw(&mycbuf), buffer);
}

TEST_F(cbufTest, Full)
{
    int arr[ecount];
    for (size_t e = 0; e < ecount; e++)
    {
        arr[e] = e;
        cbuf_put(&mycbuf, &e);
    }

    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_len(&mycbuf), ecount);
    ASSERT_EQ(cbuf_avail(&mycbuf), 0);

    int e = 100;
    int ret = 0;
    ret = cbuf_put(&mycbuf, &e);

    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_len(&mycbuf), ecount);
    ASSERT_EQ(cbuf_avail(&mycbuf), 0);

    int earr[8] = {100, 101, 102, 103, 104, 105, 106, 107};
    ret = cbuf_write(&mycbuf, earr, 8);

    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(cbuf_is_empty(&mycbuf));
    ASSERT_EQ(cbuf_len(&mycbuf), ecount);
    ASSERT_EQ(cbuf_avail(&mycbuf), 0);
}
