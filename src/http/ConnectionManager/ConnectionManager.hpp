#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <filesystem>
#include <sys/epoll.h>

#include "RawRequest.hpp"
#include "ClientState.hpp"
#include "StrUtils.hpp"
#include "Config.hpp"
#include "NetworkEndpoint.hpp"
#include "RequestHandler.hpp"
#include "FileUtils.hpp"
#include "Client.hpp"
#include "CgiRequestResult.hpp"
#include "PrintUtils.hpp"
#include "debug.hpp"

class Server;

class ConnectionManager
{
  private:
    // Properties
    const Config& m_config;
    std::unordered_map<int, ClientState> m_clients;

    // Methods
    size_t processReqs(Client& client, const std::string& tcpData);
    void genResps(Client& client);

  public:
    // Construction and destruction
    ConnectionManager() = delete;
    ConnectionManager(const Config& config);
    ~ConnectionManager() = default;
    ConnectionManager(const ConnectionManager&) = default;
    ConnectionManager& operator=(const ConnectionManager&) = delete;
    ConnectionManager(ConnectionManager&&) noexcept = default;
    ConnectionManager& operator=(ConnectionManager&&) noexcept = delete;

    // Accessors
    ClientState& clientState(int clientId);

    // Methods
    void addClient(int clientId);
    void removeClient(int clientId);
    void processData(Client& client, const std::string& tcpData);
    CGIData* findCgiByStdoutFd(int fd);
    CGIData* findCgiByStdinFd(int fd);
    void onCgiExited(Server& server, pid_t pid, int status);
};

#endif
