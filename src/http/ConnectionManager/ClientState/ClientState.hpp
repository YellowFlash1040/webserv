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
#include "debug.hpp"

class ClientState
{
  private:
    // Raw requests from the client (byte level)
    std::deque<RawRequest> _rawRequests;

    // Raw responses (not yet serialized), allows handling internal redirects
    std::deque<RawResponse> _rawResponsesQueue;

    // Responses ready to be sent on the socket
    std::queue<ResponseData> _respDataQueue;

  public:
    ClientState();
    ~ClientState() = default;
    ClientState(const ClientState& other) = default;
    ClientState& operator=(const ClientState& other) = default;
    ClientState(ClientState&& other) noexcept = default;
    ClientState& operator=(ClientState&& other) noexcept = default;

    // size_t getRawReqCount() const;

    RawRequest& getRawRequest(size_t idx);
    const RawRequest& getRawRequest(size_t idx) const;

    RawRequest& getLatestRawReq();
    const RawRequest& getLatestRawReq() const;

    size_t getLatestRawReqIndex() const;

    // bool latestRawReqNeedsBody() const;

    void enqueueResponseData(const ResponseData& resp);

    const ResponseData& getRespDataObj() const;

    RawRequest& addRawRequest();

    RawRequest popRawReq();

    bool hasPendingResponseData() const;

    bool hasCompleteRawRequest() const;
    RawRequest popFirstCompleteRawRequest();

    ResponseData& frontResponseData();
    void popFrontResponseData();

    RawResponse& peekLastRawResponse();
    RawResponse popNextRawResponse();

    const std::queue<ResponseData>& getResponseQueue() const
    {
        return _respDataQueue;
    }
};

#endif
