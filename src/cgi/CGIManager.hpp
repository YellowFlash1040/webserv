#pragma once

#ifndef CGIMANAGER_HPP
# define CGIMANAGER_HPP

# include "Client.hpp"
# include "RequestData.hpp"
# include "CGIData.hpp"
# include "debug.hpp"

class CGIManager
{
  public:
    // Construction
    CGIManager() = delete;
    ~CGIManager() = delete;

    // Methods
    static CGIData startCGI(const RequestData& req, Client& client,
                            const std::string& interpreter,
                            const std::string& scriptPath);

  private:
    // Methods
    static std::vector<std::string> buildEnvFromRequest(
        const RequestData& req, Client& client, const std::string& scriptPath);
    static void addEnv(std::vector<std::string>& env, const std::string& key,
                       const std::string& value);
    static void addRequestInfo(std::vector<std::string>& env,
                               const Client& client, const RequestData& req,
                               const std::string& scriptPath);
    static void addReqHeaders(std::vector<std::string>& env,
                              const RequestData& req);
    static void addServerInfo(std::vector<std::string>& env,
                              const RequestData& req, const Client& client);
    static void addServerName(std::vector<std::string>& env,
                              const RequestData& req, const Client& client);
    static void addServerAddr(std::vector<std::string>& env,
                              const Client& client);
    static void addRemoteAddr(std::vector<std::string>& env,
                              const Client& client);
};

#endif
