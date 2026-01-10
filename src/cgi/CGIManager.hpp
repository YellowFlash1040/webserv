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
    CGIManager() = default;
    ~CGIManager() = default;

    // Methods
    static CGIData startCGI(const RequestData& req, Client& client,
                            const std::string& interpreter,
                            const std::string& scriptPath);

  private:
    // Methods
    static std::vector<std::string> buildEnvFromRequest(
        const RequestData& req, Client& client, const std::string& scriptPath);
};

#endif
