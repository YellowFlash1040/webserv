#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <queue>
#include <vector>

#include "../../Request/RawRequest/RawRequest.hpp"
#include "../../Request/RequestData/RequestData.hpp"
#include "../../HttpMethod/HttpMethod.hpp"
#include "../../Response/RawResponse/RawResponse.hpp"
#include "debug.hpp"

class ClientState
{
  private:
    // Properties
    // Raw requests from the client (byte level)
    std::vector<RawRequest> _rawRequests;

    // Raw responses (not yet serialized), allows handling internal redirects
    std::queue<RawResponse> _rawResponsesQueue;

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
    void enqueueResponseData(const ResponseData& resp);
    RawRequest& addRawRequest();
    RawRequest popFirstCompleteRawRequest();
    void popFrontResponseData();
};

#endif
