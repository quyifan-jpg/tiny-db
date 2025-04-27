#include "db_impl.h"
#include "../cache/cache.h"

namespace smallkv {
    DBImpl::DBImpl(Options options) : options_(std::move(options)) {
        logger = log::get_logger();
        cache = std::make_shared<Cache<std::string, std::string>>(options_.CACHE_SIZE);
    }

    DBStatus DBImpl::Put(const WriteOptions &options,
                         const std::string_view &key,
                         const std::string_view &value) {
        std::unique_lock<std::shared_mutex> wlock(rwlock_);
        if (key.empty() || value.empty()) {
            return Status::InvalidArgs;
        }
        // here we have a copy of key and value
        cache->insert(key.data(), new std::string(value));
        
        return Status::Success;
    }

    DBStatus DBImpl::Delete(const WriteOptions &options,
                            const std::string_view &key) {
        return Status::NotImpl;
    }

    DBStatus DBImpl::Get(const ReadOptions &options,
                         const std::string_view &key,
                         std::string *ret_value_ptr) {
        std::shared_lock<std::shared_mutex> rlock(rwlock_);

        if (cache->contains(key.data())) {
            *ret_value_ptr = *(cache->get(key.data())->val);
            logger->info("Cache hit.");
            return Status::Success;
        }
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