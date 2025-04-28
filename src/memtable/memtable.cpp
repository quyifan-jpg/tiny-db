
#include "memtable.h"
#include "memory_pool/default_allocator.h"
#include "log/log.h"
namespace smallkv {
    MemTable::MemTable(std::shared_ptr<FreeListAllocate> alloc)
        : alloc_(std::move(alloc)) {
        // log_debug("MemTable created with allocator: {}", alloc_.get());
        logger_ = log::get_logger();
    }
    
}