#include <string>
#include <cstdint>

#ifndef SMALLKV_CODEC_H
#define SMALLKV_CODEC_H
namespace smallkv::utils {
    // 编解码
    void EncodeFixed8(char *buf, uint8_t val){
        buf[0] = val & 0xff;
    };

    void EncodeFixed32(char *buf, uint32_t val){
        buf[0] = val & 0xff;
        buf[1] = (val >> 8) & 0xff;
        buf[2] = (val >> 16) & 0xff;
        buf[3] = (val >> 24) & 0xff;
    };

    void EncodeFixed64(char *buf, uint64_t val){
        buf[0] = val & 0xff;
        buf[1] = (val >> 8) & 0xff;
        buf[2] = (val >> 16) & 0xff;
        buf[3] = (val >> 24) & 0xff;
        buf[4] = (val >> 32) & 0xff;
        buf[5] = (val >> 40) & 0xff;
        buf[6] = (val >> 48) & 0xff;
        buf[7] = (val >> 56) & 0xff;
    };

    uint8_t DecodeFixed8(const char *data){
        return *reinterpret_cast<const uint8_t*>(data);
    };

    uint32_t DecodeFixed32(const char *data){
        auto _data = reinterpret_cast<const uint8_t *>(data);
        return static_cast<uint32_t>(_data[0]) |
               (static_cast<uint32_t>(_data[1]) << 8) |
               (static_cast<uint32_t>(_data[2]) << 16) |
               (static_cast<uint32_t>(_data[3]) << 24);
    }
;

    uint64_t DecodeFixed64(const char *data){
        auto _data = reinterpret_cast<const uint8_t *>(data);
        return static_cast<uint64_t>(_data[0]) |
            (static_cast<uint64_t>(_data[1]) << 8) |
            (static_cast<uint64_t>(_data[2]) << 16) |
            (static_cast<uint64_t>(_data[3]) << 24) |
            (static_cast<uint64_t>(_data[4]) << 32) |
            (static_cast<uint64_t>(_data[5]) << 40) |
            (static_cast<uint64_t>(_data[6]) << 48) |
            (static_cast<uint64_t>(_data[7]) << 56);
    };

    inline void PutFixed8(std::string &dst, uint8_t val) {
        char buf[sizeof(val)];
        EncodeFixed8(buf, val);
        dst.append(buf, sizeof(val));
    }

    inline void PutFixed32(std::string &dst, uint32_t val) {
        char buf[sizeof(val)];
        EncodeFixed32(buf, val);
        dst.append(buf, sizeof(val));
    }

    inline void PutFixed64(std::string &dst, uint64_t val) {
        char buf[sizeof(val)];
        EncodeFixed64(buf, val);
        dst.append(buf, sizeof(val));
    }

    // 构建形如"level_n_sst_i.sst"的文件名，其中n是level层数，i是该层的第i个sst文件
    inline std::string BuildSSTPath(uint32_t n, uint32_t i) {
        return "level_" + std::to_string(n) + "_sst_" + std::to_string(i) + ".sst";
    }
}
#endif //SMALLKV_CODEC_H
