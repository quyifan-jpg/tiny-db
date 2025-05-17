#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include "db/db.h"
#include "db/options.h"

int main() {
  const int N = 100000;
  auto opts = smallkv::MakeOptionsForDebugging();
  auto db = std::make_unique<smallkv::DB>(opts);

  // 准备数据
  std::vector<std::string> keys, vals;
  keys.reserve(N); vals.reserve(N);
  for (int i = 0; i < N; i++) {
    keys.push_back("key_" + std::to_string(i));
    vals.push_back("val_" + std::to_string(i));
  }

  // 测 Put
  auto t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; i++) {
    db->Put(smallkv::WriteOptions(), keys[i], vals[i]);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  double put_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
  std::cout << "Put " << N << " ops in " << put_ms << " ms, "
            << (N / put_ms * 1000) << " ops/s\n";

  // 预写入并测 Get
  // （为了公平测，重启实例）
  db->Close();
  db = std::make_unique<smallkv::DB>(opts);
  for (int i = 0; i < N; i++) db->Put(smallkv::WriteOptions(), keys[i], vals[i]);

  t1 = std::chrono::high_resolution_clock::now();
  std::string out;
  for (int i = 0; i < N; i++) {
    db->Get(smallkv::ReadOptions(), keys[i], &out);
    out.clear();
  }
  t2 = std::chrono::high_resolution_clock::now();
  double get_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
  std::cout << "Get " << N << " ops in " << get_ms << " ms, "
            << (N / get_ms * 1000) << " ops/s\n";

  return 0;
}
