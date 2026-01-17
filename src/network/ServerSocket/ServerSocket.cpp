#include "ServerSocket.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

// Default constructor
ServerSocket::ServerSocket(const NetworkEndpoint& endpoint, int queueSize)
  : Socket()
  , m_endpoint(endpoint)
{
    Socket::setNonBlockingAndCloexec(m_fd);

    int opt = 1;
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt");

    t_sockaddr_in addr;
    fillAddressInfo(addr, endpoint);

    if (bind(m_fd, reinterpret_cast<t_sockaddr*>(&addr), sizeof(addr)) == -1)
        throw std::runtime_error("bind");
    if (listen(m_fd, queueSize) == -1)
        throw std::runtime_error("listen");
}

// Move constructor
ServerSocket::ServerSocket(ServerSocket&& other) noexcept
  : Socket(std::move(other))
  , m_endpoint(other.m_endpoint)
{
}

// Move assignment operator
ServerSocket& ServerSocket::operator=(ServerSocket&& other) noexcept
{
    if (this != &other)
    {
        Socket::operator=(std::move(other));
        m_endpoint = other.m_endpoint;
    }
    return (*this);
}

// Destructor
ServerSocket::~ServerSocket() {}

// ---------------------------METHODS-----------------------------

NetworkEndpoint& ServerSocket::endpoint()
{
    return m_endpoint;
}

const NetworkEndpoint& ServerSocket::endpoint() const
{
    return m_endpoint;
}

void ServerSocket::fillAddressInfo(t_sockaddr_in& addr,
                                   const NetworkEndpoint& e)
{
    ft::bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(e.port());
    addr.sin_addr.s_addr = htonl(static_cast<uint32_t>(e.ip()));
}
