#pragma once

#ifndef FDGUARD_HPP
# define FDGUARD_HPP

#include <unistd.h>

class FdGuard
{
public:
    explicit FdGuard(int fd = -1) : _fd(fd) {}

    ~FdGuard(){ if (_fd != -1) close(_fd);}

    int get() const { return _fd; }

    int release() { int tmp = _fd; _fd = -1; return tmp;}

private:
    int _fd;
};

#endif