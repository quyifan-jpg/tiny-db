
#include "default_allocator.h"

namespace smallkv {
    void *DefaultAlloc::Allocate(int32_t n) {
        return malloc(n);
    }

    void DefaultAlloc::Deallocate(void *p, int32_t n) {
        free(p);
    }

    void *DefaultAlloc::Reallocate(void *p, int32_t old_size, int32_t new_size) {
        return realloc(p, new_size);
    }
}