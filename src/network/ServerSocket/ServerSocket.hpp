#pragma once

#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

# include <utility>

# include "Socket.hpp"
# include "NetworkEndpoint.hpp"
# include "MemoryUtils.hpp"

typedef struct sockaddr t_sockaddr;
typedef struct sockaddr_in t_sockaddr_in;

class ServerSocket : public Socket
{
    // Construction and destruction
  public:
    ServerSocket(const NetworkEndpoint& endpoint, int queueSize);
    ServerSocket(const ServerSocket& other) = delete;
    ServerSocket& operator=(const ServerSocket& other) = delete;
    ServerSocket(ServerSocket&& other) noexcept;
    ServerSocket& operator=(ServerSocket&& other) noexcept;
    ~ServerSocket();

    // Class specific features
    NetworkEndpoint& endpoint();
    const NetworkEndpoint& endpoint() const;

  private:
    // Properties
    NetworkEndpoint m_endpoint;
    // Methods
    void fillAddressInfo(t_sockaddr_in& addr, const NetworkEndpoint& e);
};

#endif
