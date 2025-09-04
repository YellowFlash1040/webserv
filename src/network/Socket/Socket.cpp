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

// ---------------------------ACCESSORS-----------------------------

int Socket::fd(void)
{
    return (m_fd);
}

// ---------------------------METHODS-----------------------------
