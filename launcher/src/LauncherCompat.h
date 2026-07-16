#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
namespace stdext {
    template<typename T>
    inline T* make_checked_array_iterator(T* ptr, size_t size, size_t index = 0) {
        return ptr + index;
    }
}
#endif
