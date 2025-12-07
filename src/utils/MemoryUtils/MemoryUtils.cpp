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

void bzero(void* ptr, std::size_t num)
{
    memset(ptr, 0, num);
}

} // namespace ft
