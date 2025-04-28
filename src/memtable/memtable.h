#include <memory>
#include <string>
// #include "memtable_config.h"

#include "log/log.h"
#include <optional>

#ifndef SMALLKV_MEMTABLE_H
#define SMALLKV_MEMTABLE_H

namespace smallkv {
    enum OpType {
        kDeletion = 0x1, // 删除
        kAdd = 0x2,      // 添加
        kUpdate = 0x3,   // 更新
    };
    template<typename Key, typename Value>
    class SkipList;

    class DefaultAlloc;

    class SSTableBuilder;

    class MemTableIterator;

    /*
     * Insert逻辑：
     * 1. Add key, OpType=kAdd
     *     1.1 active memtable中不存在key，正常添加即可；
     *     1.2 active memtable中存在key，进行原地更新。
     *
     * 2. Delete key, OpType=kDeletion
     *     2.1 active memtable中不存在key，相当于插入<key, nil>
     *     2.2 active memtable中存在key，原地更新为<key, nil>
     *
     *
     * */
    class MemTable final {
    public:
        explicit MemTable(std::shared_ptr<DefaultAlloc> alloc);

        ~MemTable() = default;

        MemTable() = delete;

        // 禁用拷贝、赋值。
        MemTable(const MemTable &) = delete;

        MemTable &operator=(const MemTable &) = delete;

        inline void Add(const std::string_view &key,
                        const std::string_view &value) {
            this->Insert(OpType::kAdd, key, value);
        }

        inline void Update(const std::string_view &key,
                           const std::string_view &value) {
            this->Insert(OpType::kUpdate, key, value);
        }

        inline void Delete(const std::string_view &key) {
            this->Insert(OpType::kDeletion, key, "");
        }

        // 获得memtable底层的跳表的内存占用
        int64_t GetMemUsage();

        // 获得memtable底层的跳表的key数量
        int64_t GetSize();

        bool Contains(const std::string_view &key);

        // 如果不存在则返回nullopt
        std::optional<std::string> Get(const std::string_view &key);

        // 将内存中的memtable转为磁盘中的l1 sst
        // sst_filepath格式为"/a/b/c.sst"
        void ConvertToL1SST(const std::string &sst_filepath,
                            std::shared_ptr<SSTableBuilder> sstable_builder);

        // 外部调用，创建一个MemIter，来遍历MemTable底层的跳表，本质上有跳表中的Iter提供支持
        MemTableIterator *NewIter();

    private:
        // Add、Update、Delete都属于Insert
        // 如果是Delete，则value=""
        // OpType = {kDeletion, kAdd, kUpdate}
         static constexpr int MAX_KEY_NUM = 4096;
        void Insert(OpType op_type, const std::string_view &key,
                    const std::string_view &value);

        // 在leveldb中学到的设计模式：声明一个友元迭代器，然后提供一个NewIter的public方法给外部创建迭代器
        friend class MemTableIterator;

    private:
        std::shared_ptr<SkipList<std::string, std::string>> ordered_table_;
        std::shared_ptr<DefaultAlloc> alloc_;
        std::shared_ptr<spdlog::logger> logger_;
    };
}

#endif //SMALLKV_MEMTABLE_H