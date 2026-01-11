#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <filesystem>

#include "../RawRequest/RawRequest.hpp"
#include "ClientState/ClientState.hpp"
#include "../utils/StrUtils.hpp"
#include "../config/Config/Config.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "../../RequestHandler/RequestHandler.hpp"
#include "../FileUtils/FileUtils.hpp"
#include "../network/Client/Client.hpp"
#include "debug.hpp"
#include "../utils/PrintUtils.hpp"
#include "RequestResult.hpp"

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
    ConnectionManager() = delete;
    ConnectionManager(const Config& config);
    ~ConnectionManager() = default;
    ConnectionManager(const ConnectionManager&) = default;
    ConnectionManager& operator=(const ConnectionManager&) = delete;
    ConnectionManager(ConnectionManager&&) noexcept = default;
    ConnectionManager& operator=(ConnectionManager&&) noexcept = delete;

    // Accessors
    ClientState& getClientState(int clientId);
    // Methods
    void addClient(int clientId);
    void removeClient(int clientId);
    bool processData(Client& client, const std::string& tcpData);

    CGIData* findCgiByStdoutFd(int fd);
    CGIData* findCgiByStdinFd(int fd);
    void onCgiExited(pid_t pid, int status);
};

#endif
