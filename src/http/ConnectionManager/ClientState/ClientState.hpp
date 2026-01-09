#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <queue>

#include "../../Request/RawRequest/RawRequest.hpp"
#include "../../Request/RequestData/RequestData.hpp"
#include "../../HttpMethod/HttpMethod.hpp"
#include "../../Response/RawResponse/RawResponse.hpp"
#include "../../utils/debug.hpp"

class ClientState
{
  private:
    // Properties
    // Raw requests from the client (byte level)
    std::deque<RawRequest> _rawRequests;

    // Responses ready to be sent on the socket
    std::queue<ResponseData> _respDataQueue;

  public:
    ClientState();
    ~ClientState() = default;
    ClientState(const ClientState& other) = default;
    ClientState& operator=(const ClientState& other) = default;
    ClientState(ClientState&& other) noexcept = default;
    ClientState& operator=(ClientState&& other) noexcept = default;

    // Accessors
    RawRequest& getLatestRawReq();
    bool hasPendingResponseData() const;
    bool hasCompleteRawRequest() const;
    ResponseData& frontResponseData();
    const std::queue<ResponseData>& getResponseQueue() const;
    // Methods
    RawRequest& addRawRequest();
    void enqueueResponseData(const ResponseData& resp);
    RawRequest popFirstCompleteRawRequest();
    void popFrontResponseData();
};

#endif
