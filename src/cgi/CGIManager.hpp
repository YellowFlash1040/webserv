#pragma once
#include "../Request/RequestData/RequestData.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "Client.hpp"

class CGIManager
{
public:
    struct CGIData
    {
        pid_t pid = -1;
        int fd_stdout = -1;
        int fd_stdin  = -1;
        time_t start_time;
    };

    CGIManager() = default;
    ~CGIManager() = default;

    static CGIData startCGI(const RequestData& req,
                     Client& client,
                     const std::string& interpreter,
                     const std::string& scriptPath);

private:
    static std::vector<std::string> buildEnvFromRequest(const RequestData& req, Client& client, const std::string& scriptPath);
};
