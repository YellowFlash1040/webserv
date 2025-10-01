#ifndef CLIENTSTATE
#define CLIENTSTATE

#include <string>
#include <iostream>
#include <vector>
#include <cstdint> // for SIZE_MAX
#include <queue>
#include "../ParsedRequest/ParsedRequest.hpp"
#include "../ServerResponse/ServerResponse.hpp"


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
		void addParsedRequest(const ParsedRequest& req);
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
		void prepareForNextRequest();
		void finalizeLatestRequestBody();
		void prepareForNextRequestPreserveBuffers();
		
		ParsedRequest popFirstFinishedRequest();
		ParsedRequest& getParsedRequest(size_t index);

		ParsedRequest& addParsedRequest();
};


#endif
