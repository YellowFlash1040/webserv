#pragma once
#include "../Request/RequestData/RequestData.hpp"
#include "../Response/RawResponse/RawResponse.hpp"
#include "../ConnectionManager/ClientState/ClientState.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "CGIParser.hpp"
#include "CGI.hpp"

class CGIHandler
{
public:
    static void processCGI(const RequestData& req, const NetworkEndpoint& endpoint, const std::string& interpreter, const std::string& scriptPath, RawResponse& rawResp);
private:
    static std::string handleCGI(const RequestData& req, const NetworkEndpoint& endpoint, const std::string& interpreter, const std::string& scriptPath);
};
