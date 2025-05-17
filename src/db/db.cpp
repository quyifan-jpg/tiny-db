//
// Created on 2023/1/28.
//
#include "db.h"
#include "db_impl.h" // Include the full definition of DBImpl

namespace smallkv
{
    // Constructor: Initialize the unique_ptr using make_unique
    DB::DB(const Options &options) : impl_(std::make_unique<DBImpl>(options)) {}

    // Destructor: Define it here where DBImpl is a complete type.
    // The unique_ptr (impl_) will automatically delete the DBImpl object
    // when the DB object is destroyed. Using = default is sufficient.
    DB::~DB() = default;

    DBStatus DB::Put(const WriteOptions &options,
                     const std::string_view &key,
                     const std::string_view &value)
    {
        // Forward the call to the implementation object
        return impl_->Put(options, key, value);
    }

    DBStatus DB::Delete(const WriteOptions &options,
                        const std::string_view &key)
    {
        return impl_->Delete(options, key);
    }

    DBStatus DB::Get(const ReadOptions &options,
                     const std::string_view &key,
                     std::string *value)
    {
        return impl_->Get(options, key, value);
    }

    DBStatus DB::BatchPut(const WriteOptions &options)
    {
        return impl_->BatchPut(options);
    }

    DBStatus DB::BatchDelete(const ReadOptions &options)
    {
        return impl_->BatchDelete(options);
    }

    DBStatus DB::Close()
    {
        return impl_->Close();
    }

} // namespace smallkv