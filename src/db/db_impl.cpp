#include "db_impl.h"
#include "../cache/cache.h"
#include "memtable/memtable.h"
#include "utils/codec.h"
#include "wal/wal_writer.h" 
#include "file/file_writer.h"
namespace smallkv {
    DBImpl::DBImpl(Options options) : options_(std::move(options)) {
        logger = log::get_logger();
        cache = std::make_shared<Cache<std::string, std::string>>(options_.CACHE_SIZE);
        cache->register_clean_handle([](const std::string &key, std::string *val) {
            delete val;
        });
        mem_table = std::make_shared<MemTable>(alloc);
        wal_writer = std::make_shared<WALWriter>(std::make_shared<FileWriter>(options_.WAL_DIR, true));
    }

    DBStatus DBImpl::Put(const WriteOptions &options,
                         const std::string_view &key,
                         const std::string_view &value) {
        std::unique_lock<std::shared_mutex> wlock(rwlock_);


        /* 1. 写WAL(fsync同步); todo
         * 2. 写memtable;
         * 3. 写缓存(提高读性能);
         * 4. 如果memtable超限，应该落盘，并且开启一个新的memtable;
         */

        // wal
        char buf[8 + key.size() + value.size()];
        EncodeKV(key, value, buf);
        wal_writer->AddLog(buf);
        // memtable
        if (mem_table->Contains(key)){
            mem_table->Update(key, value);
        } else {
            mem_table->Add(key, value);
        }
        if (key.empty() || value.empty()) {
            return Status::InvalidArgs;
        }
        cache->insert(std::string(key), new std::string(value));
        return Status::Success;
    }

    DBStatus DBImpl::Delete(const WriteOptions &options,
                            const std::string_view &key) {
        std::unique_lock<std::shared_mutex> wlock(rwlock_);
        if (key.empty()) {
            return Status::InvalidArgs;
        }

        return Status::NotImpl;
    }

    DBStatus DBImpl::Get(const ReadOptions &options,
                         const std::string_view &key,
                         std::string *ret_value_ptr) {
        std::shared_lock<std::shared_mutex> rlock(rwlock_);
        if (key.empty() || ret_value_ptr == nullptr) {
            return Status::InvalidArgs;
        }
        auto node = cache->get(std::string(key));
        if (node != nullptr) {
            ret_value_ptr->assign(*(node->val));
            cache->release(std::string(key)); 
            node = nullptr; // 清空指针，避免悬空指针
            return Status::Success;
        } 
        // if not found in cache, we can try to read from memtable or sst files
        
        // return Status::Success;
        return Status::NotFound;
    }

    DBStatus DBImpl::BatchPut(const WriteOptions &options) {
        return Status::NotImpl;
    }

    DBStatus DBImpl::BatchDelete(const ReadOptions &options) {
        return Status::NotImpl;
    }

        void DBImpl::EncodeKV(const std::string_view &key,
                          const std::string_view &value,
                          char *buf) {
        /*
         * 暂时采用的编码方法如下：
         *     +-------------+-----+-------------+-----+
         *     | key_len(4B) | key | val_len(4B) | val |
         *     +-------------+-----+-------------+-----+
         * todo: 存在优化空间，例如使用variant等，后续有空再说
         *
         * */
        assert(value.size() < UINT32_MAX);
        utils::EncodeFixed32(buf, key.size());
        memcpy(buf + 4, key.data(), key.size());
        utils::EncodeFixed32(buf + 4 + key.size(), value.size());
        memcpy(buf + 4 + key.size() + 4, value.data(), value.size());
    }

    void DBImpl::MemTableToSST() {}

    DBStatus DBImpl::Close() {
        return Status::Success;
    }
}