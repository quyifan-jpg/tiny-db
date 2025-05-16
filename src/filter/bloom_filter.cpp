
#include <iostream>

#include "bloom_filter.h"
#include <cmath>

namespace smallkv {
        uint32_t murmur_hash2(const void *key, uint32_t len) {
        // from http://www.geekhideout.com/murmur3/
        // 'm' and 'r' are mixing constants generated offline.
        // They're not really 'magic', they just happen to work well.
        static constexpr uint32_t seed = 0xbc451d34;
        static constexpr uint32_t m = 0x5bd1e995;
        static constexpr uint32_t r = 24;

        // Initialize the hash to a 'random' value

        uint32_t h = seed ^ len;

        // Mix 4 bytes at a time into the hash

        const uint8_t *data = (const unsigned char *) key;

        while (len >= 4) {
            uint32_t k = *(uint32_t *) data;

            k *= m;
            k ^= k >> r;
            k *= m;

            h *= m;
            h ^= k;

            data += 4;
            len -= 4;
        }

        // Handle the last few bytes of the input array

        switch (len) {
            case 3:
                h ^= data[2] << 16;
            case 2:
                h ^= data[1] << 8;
            case 1:
                h ^= data[0];
                h *= m;
        };

        // Do a few final mixes of the hash to ensure the last few
        // bytes are well-incorporated.

        h ^= h >> 13;
        h *= m;
        h ^= h >> 15;

        return h;
    }

    BloomFilter::BloomFilter(int32_t keys_num, double false_positive) {
        // 计算出最佳的位数组大小
        // log_2(false_positive) = -0.4804530139182014
        int32_t bits_num = -1 * static_cast<int32_t>(std::log(false_positive) * keys_num / 0.4804530139182014);
        bits_array.resize((bits_num + 7) / 8);
        bits_num = static_cast<int>(bits_array.size()) * 8; // 注意此处
        bits_per_key = bits_num / keys_num;
        // 计算最佳的哈希函数数量
        hash_func_num = static_cast<int32_t>(0.6931471805599453 * bits_per_key);
        // 保证哈希函数在[1,32]的范围内，防止过大或者过小
        if (hash_func_num < 1) {
            hash_func_num = 1;
        }
        if (hash_func_num > 32) {
            hash_func_num = 32;
        }
    }

    void BloomFilter::create_filter(const std::vector<std::string> &keys) {
        uint32_t bits_size = bits_array.size() * 8; // 位数组的长度
        for (const auto &key: keys) {
            uint32_t h = murmur_hash2(key.c_str(), key.size());
            uint32_t delta = (h >> 17) | (h << 15); // 高17位和低15位交换
            // 模拟计算hash_func_num次哈希
            for (int j = 0; j < hash_func_num; ++j) {
                uint32_t bit_pos = h % bits_size;
                bits_array[bit_pos / 8] |= (1 << (bit_pos % 8));
                // 每轮循环h都加上一个delta，就相当于每一轮都进行了一次hash
                h += delta;
            }
        }
    }

    bool BloomFilter::exists(const std::string_view &key) {
        if (key.empty()) {
            return false;
        }
        uint32_t bits_size = bits_array.size() * 8; // 位数组的长度
        uint32_t h = murmur_hash2(key.data(), key.size());
        uint32_t delta = (h >> 17) | (h << 15);
        for (int j = 0; j < hash_func_num; ++j) {
            uint32_t bit_pos = h % bits_size;
            if ((bits_array[bit_pos / 8] & (1 << (bit_pos % 8))) == 0) {
                return false;
            }
            h += delta;
        }
        return true;
    }

    uint64_t BloomFilter::size() {
        return bits_array.size();
    }

    std::string BloomFilter::policy_name() {
        return "BloomFilter";
    }

    std::string &BloomFilter::data() {
        return bits_array;
    }

    void BloomFilter::create_filter2(int32_t hash_func_num_, std::string &bits_array_) {
        hash_func_num = hash_func_num_;
        bits_array = bits_array_;
    }

    void BloomFilter::add(const std::string_view &key) {
        if (key.empty()) return;
        uint32_t bits_size = bits_array.size() * 8;
        // 初始 hash
        uint32_t h = murmur_hash2(key.data(), key.size());
        // 生成增量 delta
        uint32_t delta = (h >> 17) | (h << 15);
        // k 次哈希
        for (int j = 0; j < hash_func_num; ++j) {
        uint32_t bit_pos = h % bits_size;
        bits_array[bit_pos / 8] |= char(1 << (bit_pos % 8));
        h += delta;
        }
  }
};