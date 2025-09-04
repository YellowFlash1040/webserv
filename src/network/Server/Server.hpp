#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <utility>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <unistd.h>
# include <sys/epoll.h>
# include <fcntl.h>
# include <errno.h>
# include <cstdlib>
# include <stdexcept>

# include "MemoryUtils.hpp"

typedef struct sockaddr t_sockaddr;
typedef struct sockaddr_in t_sockaddr_in;
typedef struct epoll_event t_event;

class Server
{
    // Construction and destruction
  public:
    Server(int port);
    ~Server();

    // Class specific features
  public:
    // Accessors
    // Methods
    void run(void);

  protected:
    // Properties
    // Methods

  private:
    // constants
    static constexpr int QUEUE_SIZE = 100;
    static constexpr int MAX_EVENTS = 50;
    // Properties
    int m_socket = -1;
    int m_epfd = -1; // event poll fd
    t_sockaddr_in m_address;
    // Methods
    void fillAddressInfo(int port);
    void setNonBlockingAndCloexec(int fd);
    void Server::addSocketToEPoll(int socket, uint32_t events);
};

#endif
