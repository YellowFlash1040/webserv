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
# include <csignal>
# include <unordered_set>

# include "MemoryUtils.hpp"
# include "NetworkEndpoint.hpp"
# include "ServerSocket.hpp"

typedef struct sockaddr t_sockaddr;
typedef struct sockaddr_in t_sockaddr_in;
typedef struct epoll_event t_event;

extern volatile std::sig_atomic_t g_running;

class Server
{
    // Construction and destruction
  public:
    Server(NetworkEndpoint endpoint);
    ~Server();

    // Class specific features

  public:
    // Constants
    static constexpr int QUEUE_SIZE = 100;
    static constexpr int MAX_EVENTS = 50;
    // Accessors
    // Methods
    void run(void);
    void addEndpoint(const NetworkEndpoint& endpoint,
                     int queueSize = QUEUE_SIZE);

  protected:
    // Properties
    // Methods

  private:
    // Properties
    int m_listeningSocket = -1;
    int m_epfd = -1; // event poll fd
    //
    std::unordered_set<ServerSocket> m_listeners;
    // Methods
    void fillAddressInfo(NetworkEndpoint e);
    void setNonBlockingAndCloexec(int fd);
    void addSocketToEPoll(int socket, uint32_t events);
    void acceptNewClient(int listeningSocket);
    void processClient(int clientSocket);
};

#endif
