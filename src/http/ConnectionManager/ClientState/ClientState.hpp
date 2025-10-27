#ifndef CLIENTSTATE
#define CLIENTSTATE

#include "../RawRequest/RawRequest.hpp"
#include "../../Response/Response.hpp"
#include "../../HttpMethod/HttpMethod.hpp"
#include "../../RequestData/RequestData.hpp"

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
		std::vector<RawRequest> _rawRequests;
		std::vector<RequestData> _requestsData;
		std::queue<Response> _responseQueue;
		
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

		// Response management
		void enqueueResponse(const Response& resp);
		bool responseQueueEmpty() const;
	
		const Response& getRespObj() const;

		

		RawRequest& addRawRequest();
		

		RawRequest popRawReq();
		RequestData popRequestData();
		
		bool hasPendingResponses() const;
        Response popNextResponse();
		
		void addRequestData(const RequestData& requestData);
		
		bool hasCompleteRawRequest() const;
		RawRequest popFirstCompleteRawRequest();

};


#endif
