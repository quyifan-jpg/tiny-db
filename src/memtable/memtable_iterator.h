#ifndef SMALLKV_MEMTABLE_ITERATOR_H
#define SMALLKV_MEMTABLE_ITERATOR_H

#include <string> // Or include specific Key/Value types if known

namespace smallkv {

// Forward declaration if Key/Value types are complex
// class Key;
// class Value;

template <typename Key, typename Value>
class MemTableIterator {
public:
    virtual ~MemTableIterator() = default;

    // Returns true if the iterator is positioned at a valid node.
    virtual bool Valid() const = 0;

    // Returns the key at the current position.
    // REQUIRES: Valid()
    virtual const Key& key() const = 0;

    // Returns the value at the current position.
    // REQUIRES: Valid()
    virtual const Value& value() const = 0;

    // Moves to the next entry.
    // REQUIRES: Valid()
    virtual void Next() = 0;

    // Moves to the previous entry. (Optional, add if needed)
    // virtual void Prev() = 0;

    // Positions at the first entry in the source.
    // Final state of iterator is Valid() iff source is not empty.
    virtual void SeekToFirst() = 0;

    // Positions at the last entry in the source. (Optional, add if needed)
    // Final state of iterator is Valid() iff source is not empty.
    // virtual void SeekToLast() = 0;

    // Positions at the first entry with a key >= target.
    // Final state of iterator is Valid() iff such entry exists.
    virtual void Seek(const Key& target) = 0;

    // No copying/assignment
    MemTableIterator(const MemTableIterator&) = delete;
    MemTableIterator& operator=(const MemTableIterator&) = delete;

protected:
    // Protected constructor for derived classes
    MemTableIterator() = default;
};

} // namespace smallkv

#endif // SMALLKV_MEMTABLE_ITERATOR_H