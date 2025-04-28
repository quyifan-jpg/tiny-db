#include "db_impl.h"
#include "../cache/cache.h"
#include "memtable/memtable.h"
namespace smallkv {
    DBImpl::DBImpl(Options options) : options_(std::move(options)) {
        logger = log::get_logger();
        cache = std::make_shared<Cache<std::string, std::string>>(options_.CACHE_SIZE);
        cache->register_clean_handle([](const std::string &key, std::string *val) {
            delete val;
        });
        mem_table = std::make_shared<MemTable>(alloc);
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
                          char *buf) {}

    void DBImpl::MemTableToSST() {}

    DBStatus DBImpl::Close() {
        return Status::Success;
    }
}