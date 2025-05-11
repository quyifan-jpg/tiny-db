#include <memory>
#include <string_view>
#include <utility>
#include "db/status.h"
#include "file/file_reader.h"
#include "utils/log_format.h"

#ifndef SMALLKV_WAL_READER_H
#define SMALLKV_WAL_READER_H
namespace smallkv {
    class WALReader final {
    public:
        // 读取的文件路径
        explicit WALReader(std::shared_ptr<FileReader> readableFile) : readableFile(std::move(readableFile)), offset_(0) {}

        ~WALReader();

        // 读取一条log记录
        // 返回: Status::Success 表示成功读取一条记录
        //       Status::EndOfFile 表示文件结束
        //       其他状态表示错误
        DBStatus ReadLog(std::string* log);

        // 验证checksum
        static bool VerifyChecksum(const std::string_view& data, uint32_t expected_checksum);

    private:
        std::shared_ptr<FileReader> readableFile;
        size_t offset_;
    };
}
#endif 