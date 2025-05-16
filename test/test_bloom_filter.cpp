
#include <memory>
#include <gtest/gtest.h>
#include "../src/filter/bloom_filter.h"
#include "../src/filter/filter_policy.h"

namespace smallkv::unittest
{
    TEST(BloomFilterTest, BasicInsertAndQuery)
    {
        // 准备
        std::vector<std::string> keys = {
            "apple", "banana", "cherry", "date", "elderberry"};
        // 预计插入 5 个 key，允许 1% 的假阳性率
        BloomFilter filter(/*keys_num=*/5, /*false_positive=*/0.01);

        // 插入
        filter.create_filter(keys);

        // 容量和哈希数都应该大于 0
        EXPECT_GT(filter.size(), 0u);
        EXPECT_GE(filter.get_hash_num(), 1);

        // 已插入的 key 一定存在
        for (const auto &k : keys)
        {
            EXPECT_TRUE(filter.exists(k))
                << "Inserted key \"" << k << "\" should exist";
        }

        // 一批肯定没插入的 key，应该不存在（可能偶发假阳性，但这里冲突极小）
        std::vector<std::string> negatives = {
            "fig", "grape", "honeydew", "kiwi", "lemon"};
        for (const auto &k : negatives)
        {
            EXPECT_FALSE(filter.exists(k))
                << "Non-inserted key \"" << k << "\" false-positive";
        }
    }
    TEST(BloomFilter, basic)
    {
        std::vector<std::string> data;
        for (int i = 0; i < 10 * 10000; ++i)
        {
            data.push_back("key_" + std::to_string(i));
        }
        // 预期10w条数据，假阳性概率为1%
        std::unique_ptr<FilterPolicy> filterPolicy = std::make_unique<BloomFilter>(10 * 10000, 0.01);
        filterPolicy->create_filter(data);
        int cnt = 0;
        for (int i = 0; i < 10 * 10000; ++i)
        {
            if (filterPolicy->exists("key_" + std::to_string(i)))
            {
                ++cnt;
            }
        }
        EXPECT_EQ(10 * 10000, cnt);
        cnt = 0;
        for (int i = 10 * 10000; i < 20 * 10000; ++i)
        {
            if (filterPolicy->exists("key_" + std::to_string(i)))
            {
                ++cnt;
            }
        }
        EXPECT_LT(cnt, 3000); // cnt预期是1000（10条数据，假阳性概率为1%，预期为1000）
    }

    TEST(BloomFilter, basic2)
    {
        std::vector<std::string> data;
        for (int i = 0; i < 100 * 10000; ++i)
        {
            data.push_back("key_" + std::to_string(i));
        }
        std::unique_ptr<FilterPolicy> filterPolicy = std::make_unique<BloomFilter>(100 * 10000, 0.001);
        filterPolicy->create_filter(data);
        int cnt = 0;
        for (int i = 0; i < 100 * 10000; ++i)
        {
            if (filterPolicy->exists("key_" + std::to_string(i)))
            {
                ++cnt;
            }
        }
        EXPECT_EQ(100 * 10000, cnt);
        cnt = 0;
        for (int i = 100 * 10000; i < 200 * 10000; ++i)
        {
            if (filterPolicy->exists("key_" + std::to_string(i)))
            {
                ++cnt;
            }
        }
        EXPECT_LT(cnt, 3000);
    }
}