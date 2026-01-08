#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <unordered_map>
# include <stdexcept>
# include <sys/epoll.h>
# include <errno.h>
# include <csignal> // for g_running
# include <unordered_set>
# include <vector>
# include <memory>
# include <sys/timerfd.h>
# include <sys/wait.h>

# include "Client.hpp"
# include "Config.hpp"
# include "NetworkEndpoint.hpp"
# include "ServerSocket.hpp"
# include "ConnectionManager.hpp"
# include "ClientState.hpp"
# include "../../utils/FdGuard/FdGuard.hpp"

typedef struct epoll_event t_event;

extern volatile std::sig_atomic_t g_running;

class Server
{
    // Construction and destruction
  public:
    Server(const Config& config);
    ~Server();

    // Class specific features
  public:
    // Constants
    static constexpr int QUEUE_SIZE = 100;
    static constexpr int MAX_EVENTS = 50;
    static constexpr size_t BUFFER_SIZE = 8192;
    static constexpr size_t TIMEOUT = 60;
    // Methods
    void run(void);
    void addEndpoint(const NetworkEndpoint& endpoint);
    void removeClient(Client& client);
    void printAllClients() const;
    int createTimerFd(int interval_sec);
    void checkClientTimeouts();

  private:
    // Properties
    int m_epfd = -1; // event poll fd
    int m_timerfd = -1;
    std::unordered_map<int, ServerSocket> m_listeners;
    std::unordered_map<int, std::unique_ptr<Client>> m_clients;
    ConnectionManager m_connMgr;
    // Methods
    void addSocketToEPoll(int socket, uint32_t events);
    void acceptNewClient(int listeningSocket, int epoll_fd);
    void processClient(Client& client);
    void flushClientOutBuffer(Client& client);

    void handleCgiTermination(CGIManager::CGIData& cgi);
    void handleCgiStdin(CGIManager::CGIData& cgi);
    void handleCgiStdout(CGIManager::CGIData& cgi);
    void reapDeadCgis();
    void fillBuffer(Client& client);
};

#endif
