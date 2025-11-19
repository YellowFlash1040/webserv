#include "Socket.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
Socket::Socket()
  : m_fd(-1)
{
    m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_fd == -1)
        throw std::runtime_error("socket");
}

// Move constructor
Socket::Socket(Socket&& other) noexcept
  : m_fd(other.m_fd)
{
    other.m_fd = -1;
}

// Move assignment operator
Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        m_fd = other.m_fd;
        other.m_fd = -1;
    }
    return (*this);
}

// Destructor
Socket::~Socket()
{
    if (m_fd != -1)
        close(m_fd);
}

// -------------------------ACCESSORS-----------------------------

int Socket::fd(void) const
{
    return (m_fd);
}

// --------------------------OPERATORS----------------------------

Socket::operator int() const
{
    return (m_fd);
}

// ---------------------------METHODS-----------------------------

void Socket::setNonBlockingAndCloexec(int fd)
{
    // Non-blocking
    int status_flags = fcntl(fd, F_GETFL, 0);
    if (status_flags == -1)
        throw std::runtime_error("fcntl getfl");

    int result = fcntl(fd, F_SETFL, status_flags | O_NONBLOCK);
    if (result == -1)
        throw std::runtime_error("fcntl setfl");

    // Close-on-exec
    int fd_flags = fcntl(fd, F_GETFD, 0);
    if (fd_flags == -1)
        throw std::runtime_error("fcntl getfd");

    result = fcntl(fd, F_SETFD, fd_flags | FD_CLOEXEC);
    if (result == -1)
        throw std::runtime_error("fcntl setfd");
}
