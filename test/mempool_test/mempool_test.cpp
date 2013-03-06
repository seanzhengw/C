#include <stdio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C"
{
#include <mempool/mempool.h>
}

class mempoolTest : public ::testing::Test
{
protected:
    mempoolTest() {}
    virtual ~mempoolTest() {}
    virtual void SetUp() override
    {
        mempool_init(&pool, buffer, 4096);
    }
    virtual void TearDown() override
    {
    }

    char buffer[4096];
    struct mempool pool;
};

TEST_F(mempoolTest, Init)
{
    ASSERT_EQ(mempool_avail(&pool), 4096 - 4);
    ASSERT_EQ(mempool_has(&pool, pool.first_block), 0);
    ASSERT_EQ(mempool_has(&pool, pool.first_block + 1), 0);
    ASSERT_EQ(mempool_has(&pool, pool.first_block + 1024), 0);
    ASSERT_EQ(mempool_max_continuous(&pool), 4096 - 4);
}

TEST_F(mempoolTest, AllocAndHasAndFree)
{
    int *int_arr = (int *)mempool_alloc(&pool, 10 * sizeof(int));

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - 44);

    for (size_t i = 0; i < 10; i++)
    {
        int_arr[i] = i * 100;
    }

    int *int_arr2 = (int *)mempool_alloc(&pool, 10 * sizeof(int));

    for (size_t i = 0; i < 10; i++)
    {
        int_arr2[i] = i * 100 + 1;
    }

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - 44 - 44);

    ASSERT_EQ(mempool_has(&pool, int_arr), 1);
    ASSERT_EQ(mempool_has(&pool, int_arr + 1), 0);
    ASSERT_EQ(mempool_has(&pool, int_arr2), 1);
    ASSERT_EQ(mempool_has(&pool, int_arr2 + 1), 0);
    ASSERT_THAT(std::vector<int>(int_arr, int_arr + 10), ::testing::ElementsAre(0, 100, 200, 300, 400, 500, 600, 700, 800, 900));
    ASSERT_THAT(std::vector<int>(int_arr2, int_arr2 + 10), ::testing::ElementsAre(1, 101, 201, 301, 401, 501, 601, 701, 801, 901));

    mempool_free(&pool, int_arr);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - 44 - 4);
    ASSERT_EQ(mempool_has(&pool, int_arr), 0);
    ASSERT_EQ(mempool_has(&pool, int_arr + 1), 0);
    ASSERT_EQ(mempool_has(&pool, int_arr2), 1);
    ASSERT_EQ(mempool_has(&pool, int_arr2 + 1), 0);
    ASSERT_THAT(std::vector<int>(int_arr2, int_arr2 + 10), ::testing::ElementsAre(1, 101, 201, 301, 401, 501, 601, 701, 801, 901));

    mempool_free(&pool, int_arr2);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4);
    ASSERT_EQ(mempool_has(&pool, int_arr), 0);
    ASSERT_EQ(mempool_has(&pool, int_arr + 1), 0);
    ASSERT_EQ(mempool_has(&pool, int_arr2), 0);
    ASSERT_EQ(mempool_has(&pool, int_arr2 + 1), 0);
}

TEST_F(mempoolTest, MaxContinuous)
{
    int *int_arrs[10];
    for (size_t i = 0; i < 10; i++)
    {
        int_arrs[i] = (int *)mempool_alloc(&pool, 10 * sizeof(int));
    }

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - (44 * 10));
    ASSERT_EQ(mempool_max_continuous(&pool), 4096 - 4 - (44 * 10));

    int *big_arr = (int *)mempool_alloc(&pool, 900 * sizeof(int));

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - (44 * 10) - 3604);
    ASSERT_EQ(mempool_max_continuous(&pool), 4096 - 4 - (44 * 10) - 3604);

    mempool_free(&pool, int_arrs[1]);
    mempool_free(&pool, int_arrs[2]);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - (44 * 10) - 3604 + (44 * 2 - 4));
    ASSERT_EQ(mempool_max_continuous(&pool), (44 * 2 - 4));

    mempool_free(&pool, int_arrs[4]);
    mempool_free(&pool, int_arrs[5]);
    mempool_free(&pool, int_arrs[6]);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - (44 * 10) - 3604 + (44 * 2 - 4) + (44 * 3 - 4));
    ASSERT_EQ(mempool_max_continuous(&pool), (44 * 3 - 4));

    mempool_free(&pool, int_arrs[3]);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - (44 * 10) - 3604 + (44 * 6 - 4));
    ASSERT_EQ(mempool_max_continuous(&pool), (44 * 6 - 4));

    mempool_free(&pool, int_arrs[9]);
    mempool_free(&pool, big_arr);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - (44 * 10) + (44 * 7 - 4));
    ASSERT_EQ(mempool_max_continuous(&pool), 3604 + 48 + 44);

    mempool_free(&pool, int_arrs[7]);
    mempool_free(&pool, int_arrs[8]);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4 - 44);
    ASSERT_EQ(mempool_max_continuous(&pool), 4096 - 4 - 44);

    mempool_free(&pool, int_arrs[0]);

    ASSERT_EQ(mempool_avail(&pool), 4096 - 4);
    ASSERT_EQ(mempool_max_continuous(&pool), 4096 - 4);
}