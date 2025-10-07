#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <queue>
#include "../ParsedRequest/ParsedRequest.hpp"
#include "../Response/Response.hpp"

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
		std::vector<ParsedRequest> _parsedRequests;
		std::queue<Response> _responseQueue;
		
	public:
		ClientState();
		~ClientState() = default;
		ClientState(const ClientState& other) = default;
		ClientState& operator=(const ClientState& other) = default;
		ClientState(ClientState&& other) noexcept = default;
		ClientState& operator=(ClientState&& other) noexcept = default;

		size_t getParsedRequestCount() const;
		
		ParsedRequest& getRequest(size_t idx);
		const ParsedRequest& getRequest(size_t idx) const;
		ParsedRequest& getLatestRequest();
		const ParsedRequest& getLatestRequest() const;
		

		size_t getLatestRequestIndex() const;
		
		bool latestRequestNeedsBody() const;

		// Response management
		void enqueueResponse(const Response& resp);
		bool responseQueueEmpty() const;
	
		const Response& getRespObj() const;

		// Ready-to-send flag

		ParsedRequest& addParsedRequest();
		
		//for gtests
		ParsedRequest popFirstFinishedReq();
};


#endif
