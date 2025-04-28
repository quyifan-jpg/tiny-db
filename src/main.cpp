#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <vector> // Include vector
#include <thread> // Include thread
#include <atomic> // Include atomic for thread-safe counters

#include "db/db.h"
#include "db/options.h"
#include "db/status.h" // Include Status definition

using namespace smallkv;

int main() {
    // 1. 初始化
    auto opts      = MakeOptionsForDebugging();
    // Ensure the DB implementation is thread-safe (assuming it is)
    auto db_holder = std::make_shared<DB>(opts); // Use shared_ptr for easier sharing
    WriteOptions wopts;
    ReadOptions  ropts;

    const int N = 10000;
    // Determine the number of threads (e.g., based on hardware or fixed)
    const unsigned int num_threads = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 4;
    std::cout << "Using " << num_threads << " threads." << std::endl;

    std::vector<std::thread> threads;
    std::atomic<bool> write_failed = false; // Flag for write errors

    // 2. 写入测试 (Multi-threaded)
    auto t1 = std::chrono::high_resolution_clock::now();

    for (unsigned int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() { // Capture necessary variables
            int start = (N / num_threads) * t;
            int end = (t == num_threads - 1) ? N : (N / num_threads) * (t + 1);

            for (int i = start; i < end; i++) {
                std::string key   = "key" + std::to_string(i);
                std::string value = "value" + std::to_string(i);
                auto status = db_holder->Put(wopts, key, value);
                if (status != Status::Success) {
                    // Use cerr for thread safety (usually) or implement proper logging
                    std::cerr << "Thread " << t << ": Put failed at i=" << i << "\n";
                    write_failed.store(true); // Signal failure
                    // Depending on requirements, you might want to stop other threads or just report
                    return; // Exit this thread's work on failure
                }
            }
        });
    }

    // Wait for all write threads to complete
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
    threads.clear(); // Clear threads vector for reuse

    auto t2 = std::chrono::high_resolution_clock::now();

    if (write_failed.load()) {
         std::cerr << "One or more write operations failed." << std::endl;
         return 1; // Exit if writing failed
    }

    auto write_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "写入 " << N << " 条总耗时 (multi-threaded): " << write_us << " μs，"
              << "平均: " << (write_us / double(N)) << " μs/条\n";


    // 3. 读取测试 (Multi-threaded)
    std::atomic<int> success_count = 0; // Use atomic for thread-safe counting
    std::atomic<int> not_found_count = 0;
    std::atomic<int> other_error_count = 0;

    auto t3 = std::chrono::high_resolution_clock::now();

    for (unsigned int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() { // Capture necessary variables
            int start = (N / num_threads) * t;
            int end = (t == num_threads - 1) ? N : (N / num_threads) * (t + 1);

            for (int i = start; i < end; i++) {
                std::string key = "key" + std::to_string(i);
                std::string value; // Each thread needs its own value buffer
                auto status = db_holder->Get(ropts, key, &value);
                if (status == Status::Success) {
                    success_count++;
                    // Optional validation (ensure thread safety if modifying shared state here)
                    // if (value != "value" + std::to_string(i)) { ... }
                } else if (status == Status::NotFound) {
                    not_found_count++;
                } else {
                    other_error_count++;
                    std::cerr << "Thread " << t << ": Get failed for key " << key << "\n";
                }
            }
        });
    }

    // Wait for all read threads to complete
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    auto t4 = std::chrono::high_resolution_clock::now();
    auto read_us = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    std::cout << "读取 " << N << " 条总耗时 (multi-threaded): " << read_us << " μs，"
              << "平均: " << (read_us / double(N)) << " μs/条\n";

    std::cout << "Get operation results:\n";
    // Load atomic values for printing
    std::cout << "  Success: " << success_count.load() << "\n";
    std::cout << "  NotFound: " << not_found_count.load() << "\n";
    std::cout << "  Other Errors: " << other_error_count.load() << "\n";

    return 0;
}
