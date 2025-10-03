#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <queue>
#include "../ParsedRequest/ParsedRequest.hpp"
#include "../ServerResponse/ServerResponse.hpp"

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
		// Requests for this client
		std::vector<ParsedRequest> _parsedRequests;

		// Responses waiting to be sent
		std::queue<ServerResponse> _responseQueue;

		bool _readyToSend;
		
	public:
		ClientState();
		~ClientState() = default;
		ClientState(const ClientState& other) = default;
		ClientState& operator=(const ClientState& other) = default;
		ClientState(ClientState&& other) noexcept = default;
		ClientState& operator=(ClientState&& other) noexcept = default;

		// Request management
		size_t getParsedRequestCount() const;
		const ParsedRequest& getReqObj(size_t index) const;
		const ParsedRequest& getLatestReqObj() const;
		ParsedRequest& getReqObj(size_t index);
		ParsedRequest& getLatestReqObj();
		bool latestRequestNeedsBody() const;

		// Response management
		void enqueueResponse(const ServerResponse& resp);
		bool responseQueueEmpty() const;
		ServerResponse popNextResponse();
		const ServerResponse& getRespObj() const;

		// Ready-to-send flag
		void setReadyToSend(bool value);
		bool isReadyToSend() const;
		
		ParsedRequest& getLatestRequest();
		void prepareNextRequestWithLeftover(const std::string& leftover);
		void finalizeLatestRequestBody();
		
		ParsedRequest& getParsedRequest(size_t index);

		ParsedRequest& addParsedRequest();
		
		//requests
		ParsedRequest& getRequest(size_t idx);
		size_t getLatestRequestIndex() const;
    
		//for gtests
		ParsedRequest popFirstFinishedRequest();
};


#endif
