#include <cassert>
#include <absl/crc/crc32c.h>
#include "file/file_writer.h"
#include "wal_reader.h"
#include "utils/log_format.h"

namespace smallkv {
    WALReader::~WALReader() {
        // todo close readableFile
        // readableFile->close();
    }

    bool WALReader::VerifyChecksum(const std::string_view &data, uint32_t expected_checksum)
    {
        auto actual_checksum = static_cast<uint32_t>(absl::ComputeCrc32c(
            absl::string_view(data.data(), data.size())));
        return actual_checksum == expected_checksum;
    }

    DBStatus WALReader::ReadLog(std::string* log) {
  assert(log != nullptr);

  // 1. 读取 header
  char header[WALConfig::kHeaderSize];
  auto s = readableFile->read(header, WALConfig::kHeaderSize, offset_);
  if (s != Status::Success) {
    return Status::ExecFailed;  // 读不到 header，视为文件尾
  }
  offset_ += WALConfig::kHeaderSize;

  // 2. 解析 checksum 和 length（小端）
  uint32_t checksum = 
      (uint8_t)header[0] |
      ((uint8_t)header[1] << 8) |
      ((uint8_t)header[2] << 16) |
      ((uint8_t)header[3] << 24);
  uint32_t len =
      (uint8_t)header[4] |
      ((uint8_t)header[5] << 8) |
      ((uint8_t)header[6] << 16) |
      ((uint8_t)header[7] << 24);

  // 3. 按 len 读取 payload
  log->resize(len);

  s = readableFile->read(log->data(), static_cast<int32_t>(len), offset_);
  if (s != Status::Success) {
    return Status::Corruption;  // 读不够字节，视为损坏
  }
  offset_ += static_cast<int32_t>(len);

  // 4. 校验 CRC
  if (!VerifyChecksum(*log, checksum)) {
    return Status::Corruption;
  }

  return Status::Success;
}
}