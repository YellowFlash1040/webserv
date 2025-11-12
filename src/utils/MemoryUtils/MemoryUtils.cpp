#include "MemoryUtils.hpp"

namespace ft
{

void* memset(void* ptr, int value, std::size_t num)
{
    unsigned char* p = static_cast<unsigned char*>(ptr);
    for (std::size_t i = 0; i < num; ++i)
        p[i] = static_cast<unsigned char>(value);
    return (ptr);
}

void* memcpy(void* dest, const void* src, std::size_t num)
{
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    for (std::size_t i = 0; i < num; ++i)
        d[i] = s[i];
    return (dest);
}

void bzero(void* ptr, std::size_t num)
{
    memset(ptr, 0, num);
}

} // namespace ft
