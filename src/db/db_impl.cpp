#include "db_impl.h"
#include "../cache/cache.h"
#include "memtable/memtable.h"
#include "utils/codec.h"
#include "wal/wal_writer.h" 
#include "file/file_writer.h"
#include "sstparser/sst_parser.h"
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

        // 1. wal
        char buf[8 + key.size() + value.size()];
        EncodeKV(key, value, buf);
        wal_writer->AddLog(buf);
        // 2. memtable
        if (mem_table->Contains(key)){
            mem_table->Update(key, value);
        } else {
            mem_table->Add(key, value);
        }
        if (key.empty() || value.empty()) {
            return Status::InvalidArgs;
        }
        // 3. cache

        cache->insert(std::string(key), new std::string(value));

        // 4. memtable超限，落盘
        if (mem_table->GetMemUsage() > options_.MEM_TABLE_MAX_SIZE) {
            // 1. 将memtable转为sst
            MemTableToSST();
            // 2. 创建新的memtable
            mem_table = std::make_shared<MemTable>(alloc);
        }
        return Status::Success;
    }

    DBStatus DBImpl::Delete(const WriteOptions &options,
                            const std::string_view &key) {
        std::unique_lock<std::shared_mutex> wlock(rwlock_);
        if (key.empty()) {
            return Status::InvalidArgs;
        }

        //  std::unique_lock<std::shared_mutex> wlock(rwlock_);

        // 1. 写WAL
        char buf[8 + key.size()]; // 用vel_len=0表示val为空
        EncodeKV(key, "", buf);
        wal_writer->AddLog(buf);

        // 2. 写memtable
        if (mem_table->Contains(key)) { // 原地标记val=""表示删除
            mem_table->Delete(key);
        } else {
            mem_table->Add(key, ""); // 墓碑机制
        }

        // 3. 删除缓存
        cache->erase(key.data());

        // 4. 检查memtable是否超限
        if (mem_table->GetMemUsage() >= options_.MEM_TABLE_MAX_SIZE) {
            MemTableToSST(); // 将memtable转为sst

            // 开启写的memtable
            mem_table = std::make_shared<MemTable>(alloc);
            logger->info("[DBImpl::Delete] A new mem_table is created.");
        }
        return Status::Success;
    }

    DBStatus DBImpl::Get(const ReadOptions &options,
                         const std::string_view &key,
                         std::string *ret_value_ptr) {
                             /*
         * 读逻辑：
         * 1. 读缓存，有则直接返回，否则进入2;
         * 2. 依次从memtable、sst文件向下查找;
         * 3. 找到的数据写入缓存;
         * 4. 返回结果;
         *
         * */
        std::shared_lock<std::shared_mutex> rlock(rwlock_);
        if (key.empty() || ret_value_ptr == nullptr) {
            return Status::InvalidArgs;
        }
        // 1. 先从缓存中读取
        if (cache->contains(key.data())) {
            *ret_value_ptr = *(cache->get(key.data())->val);
            return Status::Success;
        }
        logger->debug("[DBImpl::Get] Cache miss, try to read from memtable or sst files.");
        // if not found in cache, we can try to read from memtable or sst files
        if (mem_table->Contains(key)) {
            auto val = mem_table->Get(key);
            *ret_value_ptr = mem_table->Get(key.data()).value();
            return Status::Success;
        }
        std::string sst_path = options_.STORAGE_DIR + "level_0_sst_0.sst";
        auto sst_parser = SSTParser(sst_path);
        sst_parser.Parse();
        auto val = sst_parser.Seek(key);
        if (!val.has_value()) {
            return Status::NotFound;
        }
        ret_value_ptr->append(val.value());

        // 4. 找到的数据写入缓存
        //todo: 这里cache保存的是value的指针，然而val可能是临时值，需要优化，此处临时new 一下，性能拉胯
        auto val_ = new std::string(val.value());
        cache->insert(key.data(), val_);

        // return Status::Success;
        return Status::Success;
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