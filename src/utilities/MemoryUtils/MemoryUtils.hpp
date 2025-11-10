// MemoryUtils.hpp

#pragma once

#ifndef MEMORYUTILS_HPP
# define MEMORYUTILS_HPP

# include <cstddef> // for size_t

namespace ft
{

void* memset(void* ptr, int value, std::size_t num);
void* memcpy(void* dest, const void* src, std::size_t num);
void bzero(void* ptr, std::size_t num);

} // namespace ft

#endif
