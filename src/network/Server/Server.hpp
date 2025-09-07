#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>

# include <stdexcept>
# include <sys/epoll.h>
# include <errno.h>
# include <csignal> // for g_running
# include <unordered_set>
# include <vector>

# include "NetworkEndpoint.hpp"
# include "ServerSocket.hpp"

typedef struct epoll_event t_event;

extern volatile std::sig_atomic_t g_running;

class Server
{
    // Construction and destruction
  public:
    Server();
    ~Server();

    // Class specific features
  public:
    // Constants
    static constexpr int QUEUE_SIZE = 100;
    static constexpr int MAX_EVENTS = 50;
    // Accessors
    // Methods
    void run(void);
    void addEndpoint(const NetworkEndpoint& endpoint);

  protected:
    // Properties
    // Methods

  private:
    // Properties
    int m_epfd = -1; // event poll fd
    std::vector<ServerSocket> m_listeners;
    std::unordered_set<int> m_listenerFds;
    // Methods
    void fillAddressInfo(NetworkEndpoint e);
    void setNonBlockingAndCloexec(int fd);
    void addSocketToEPoll(int socket, uint32_t events);
    void acceptNewClient(int listeningSocket);
    void processClient(int clientSocket);
};

#endif
