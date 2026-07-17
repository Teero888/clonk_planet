#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef STDEXT_MAKE_CHECKED_ARRAY_ITERATOR_DEFINED
#define STDEXT_MAKE_CHECKED_ARRAY_ITERATOR_DEFINED
namespace stdext {
    template<typename T>
    inline T* make_checked_array_iterator(T* ptr, size_t size, size_t index = 0) {
        return ptr + index;
    }
}
#endif
#endif
