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
		
		RawRequest& addRawRequest();
		RawRequest& getLatestRawReq();
	
		void enqueueResponseData(const ResponseData& resp);

		bool hasCompleteRawRequest() const;
		RawRequest popFirstCompleteRawRequest();
		
		bool hasPendingResponseData() const;
		ResponseData& frontResponseData();
		void popFrontResponseData();
		
		// For printAllResponses only
		const std::queue<ResponseData>& getResponseQueue() const;

};

#endif
