#pragma once
#include "../Request/RequestData/RequestData.hpp"
#include "Client.hpp"
#include "ResponseData.hpp"
#include "debug.hpp"

class CGIManager
{
  public:
    struct CGIData
    {
        pid_t pid = -1;
        int fd_stdout = -1;
        int fd_stdin = -1;
        time_t start_time;
        bool addedToEpoll = false;
        std::string input;
        std::string output;
        size_t input_sent = 0;
        ResponseData* response = nullptr;
    };

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
