#ifndef CLIENTSTATE
#define CLIENTSTATE

#include "../../Request/RawRequest/RawRequest.hpp"
#include "../../Request/RequestData/RequestData.hpp"
#include "../../HttpMethod/HttpMethod.hpp"
#include "../../Response/RawResponse/RawResponse.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <queue>

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define ORANGE "\033[38;5;214m"
#define TEAL "\033[36m"
#define BLUE "\033[38;2;100;149;237m"
#define RED "\033[31m"
#define RESET "\033[0m"

class ClientState
{
	private:
  	// 1. Raw requests from the client (byte level)
	std::vector<RawRequest> _rawRequests;

	// 2. Parsed request data from RawRequest
	std::vector<RequestData> _requestsData;

	// 3. Raw responses (not yet serialized), allows handling internal redirects
	std::deque<RawResponse> _rawResponsesQueue;

	// 4. Serialized responses ready to be sent on the socket
	std::queue<ResponseData> _respDataQueue;
		
	public:
		ClientState();
		~ClientState() = default;
		ClientState(const ClientState& other) = default;
		ClientState& operator=(const ClientState& other) = default;
		ClientState(ClientState&& other) noexcept = default;
		ClientState& operator=(ClientState&& other) noexcept = default;

		size_t getRawReqCount() const;
		size_t getRequestCount() const;
		
		RawRequest& getRawRequest(size_t idx);
		const RawRequest& getRawRequest(size_t idx) const;
		RequestData& getRequest(size_t idx);
		
		RawRequest& getLatestRawReq();
		const RawRequest& getLatestRawReq() const;
		

		size_t getLatestRawReqIndex() const;
		
		bool latestRawReqNeedsBody() const;


		void enqueueResponseData(const ResponseData& resp);

	
		const ResponseData& getRespDataObj() const;

	
		RawRequest& addRawRequest();
		

		RawRequest popRawReq();
		RequestData popRequestData();
		
		bool hasPendingResponseData() const;
		ResponseData popNextResponseData();
		
		void addRequestData(const RequestData& requestData);
		
		bool hasCompleteRawRequest() const;
		RawRequest popFirstCompleteRawRequest();
		
		ResponseData& frontResponseData();
		void popFrontResponseData();

		void enqueueRawResponse(const RawResponse& resp);
		RawResponse& peekLastRawResponse();
		RawResponse popNextRawResponse();
		
		    const std::queue<ResponseData>& getResponseQueue() const { return _respDataQueue; }

};


#endif
