#include <iostream>
#include <memory>
#include <string>
#include <chrono>

#include "db/db.h"
#include "db/options.h"

using namespace smallkv;

int main() {
    // 1. 初始化
    auto opts      = MakeOptionsForDebugging();
    auto db_holder = std::make_unique<DB>(opts);
    WriteOptions wopts;
    ReadOptions  ropts;

    const int N = 100000;

    // 2. 写入测试
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        std::string key   = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        auto status = db_holder->Put(wopts, key, value);
        if (status != Status::Success) {
            std::cerr << "Put failed at i=" << i << "\n";
            return 1;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto write_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    std::cout << "写入 " << N << " 条总耗时: " << write_us << " μs，"
              << "平均: " << (write_us / double(N)) << " μs/条\n";

    // 3. 读取测试
    int success_count = 0;
    int not_found_count = 0;
    int other_error_count = 0;

    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        std::string key = "key" + std::to_string(i);
        std::string value;
        auto status = db_holder->Get(ropts, key, &value);
        if (status == Status::Success) {
            success_count++;
            // 如果需要验证：
            // if (value != "value" + std::to_string(i)) {
            //     std::cerr << "Value mismatch for key " << key << ": expected " << ("value" + std::to_string(i)) << ", got " << value << "\n";
            //     // Handle mismatch if necessary
            // }
        } else if (status == Status::NotFound) {
            not_found_count++;
            // std::cout << "Key not found: " << key << "\n"; // Optional: Log not found keys
        } else {
            other_error_count++;
            std::cerr << "Get failed for key " << key << "\n";
            // Decide if you want to continue or exit on other errors
            // return 1; // Original behavior was to exit
        }
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    auto read_us = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    std::cout << "读取 " << N << " 条总耗时: " << read_us << " μs，"
              << "平均: " << (read_us / double(N)) << " μs/条\n";

    std::cout << "Get operation results:\n";
    std::cout << "  Success: " << success_count << "\n";
    std::cout << "  NotFound: " << not_found_count << "\n";
    std::cout << "  Other Errors: " << other_error_count << "\n";

    return 0;
}
