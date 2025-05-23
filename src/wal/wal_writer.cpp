#include <cassert>
#include <absl/crc/crc32c.h>
#include "file/file_writer.h"
#include "wal_writer.h"
#include "utils/log_format.h"

namespace smallkv {
    DBStatus WALWriter::AddLog(const std::string_view &log) {
        char header[WALConfig::kHeaderSize];
        /*
         * header的schema如下：
         *         +--------------+---------+
         *         | checksum(4B) | len(4B) |
         *         +--------------+---------+
         */
        auto len = log.size();
        auto checksum = static_cast<uint32_t>(absl::ComputeCrc32c(log));// checksum
        // 小端存储：低位数据存在地址低位，比如0x1234中，34存在header[0]
        header[0] = checksum & 0xff;
        header[1] = (checksum >> 8) & 0xff;
        header[2] = (checksum >> 16) & 0xff;
        header[3] = (checksum >> 24) & 0xff;
        header[4] = len & 0xff;
        header[5] = (len >> 8) & 0xff;
        header[6] = (len >> 16) & 0xff;
        header[7] = (len >> 24) & 0xff;
        auto s = writableFile->append(header, WALConfig::kHeaderSize);
        assert(s == Status::Success);
        s = writableFile->append(log.data(), static_cast<int32_t>(len), true);

        writableFile->sync(); // 必须调用，否则不能保证持久化日志

        assert(s == Status::Success);
        return Status::Success;
    }

    WALWriter::~WALWriter() {
        writableFile->close();
    }
}
