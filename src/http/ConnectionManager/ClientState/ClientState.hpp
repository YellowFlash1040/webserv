#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <queue>

#include "RawRequest.hpp"
#include "RequestData.hpp"
#include "HttpMethod.hpp"
#include "RawResponse.hpp"
#include "CGIManager.hpp"
#include "debug.hpp"

class ClientState
{
  private:
    // Properties
    std::queue<RawRequest> m_requests;
    std::queue<ResponseData> m_responses;
    std::vector<CGIData> m_activeCGIs;

  public:
    // Construction and destruction
    ClientState();
    ClientState(const ClientState& other) = default;
    ClientState& operator=(const ClientState& other) = default;
    ClientState(ClientState&& other) noexcept = default;
    ClientState& operator=(ClientState&& other) noexcept = default;
    ~ClientState();

    // Accessors
    bool hasCompleteRequest() const;
    bool hasPendingResponse() const;
    RawRequest& backRequest();
    ResponseData& backResponse(); // the connection header is changed by CGI
    const ResponseData& frontResponse() const;
    const std::queue<ResponseData>& responses() const;
    std::vector<CGIData>& activeCGIs();

    // Methods
    RawRequest& addRequest();
    void enqueueResponse(const ResponseData& resp);
    RawRequest popFrontRequest();
    void popFrontResponse();
    CGIData& createActiveCgi(RequestData& req, Client& client,
                             const std::string& interpreter,
                             const std::string& scriptPath, ResponseData* resp);
    CGIData* findCgiByPid(pid_t pid);
    void removeCgi(pid_t pid);
    void clearActiveCGIs();
    std::vector<CGIData*> getTimedOutCGIs(time_t now, time_t timeout);
};

#endif
