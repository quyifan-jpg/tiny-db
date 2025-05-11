//
// test/test_wal_writer.cpp
//

#include <memory>
#include <cstdio>
#include <gtest/gtest.h>
#include <string>
#include "../src/file/file_writer.h"
#include "../src/file/file_reader.h"
#include "../src/wal/wal_writer.h"
#include "../src/wal/wal_reader.h"
#include "../src/db/status.h"

namespace smallkv::unittest {

TEST(wal, read_write_basic) {
    const std::string path = "./build/test/tests.wal";

    // —— 写阶段 —— 
    {
        // 1. 创建 FileWriter，并 wrap 成 shared_ptr
        auto fw = std::make_shared<FileWriter>(path, /*truncate=*/true);
        ASSERT_NE(fw, nullptr);

        // 2. 用它来构造 WALWriter
        auto writer = std::make_unique<WALWriter>(fw);
        ASSERT_NE(writer, nullptr);

        // 写几条日志
        EXPECT_EQ(writer->AddLog("log one"),       Status::Success);
        EXPECT_EQ(writer->AddLog("second message"), Status::Success);
        EXPECT_EQ(writer->AddLog("third entry!"),   Status::Success);
        // WALWriter 析构时会自动 close
    }

    // —— 读阶段 —— 
    {
        // 1. 创建 FileReader
        auto fr = std::make_shared<FileReader>(path);
        ASSERT_NE(fr, nullptr);

        // 2. 用它来构造 WALReader
        auto reader = std::make_unique<WALReader>(fr);
        ASSERT_NE(reader, nullptr);

        std::string out;
        EXPECT_EQ(reader->ReadLog(&out), Status::Success);
        EXPECT_EQ(out, "log one");

        EXPECT_EQ(reader->ReadLog(&out), Status::Success);
        EXPECT_EQ(out, "second message");

        EXPECT_EQ(reader->ReadLog(&out), Status::Success);
        EXPECT_EQ(out, "third entry!");

        // 文件结束
        EXPECT_EQ(reader->ReadLog(&out), Status::ExecFailed);
    }

    // 删除测试文件
    std::remove(path.c_str());
}

}  // namespace smallkv::unittest
