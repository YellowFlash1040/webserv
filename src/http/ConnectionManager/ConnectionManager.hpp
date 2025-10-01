#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdint>  // for SIZE_MAX
#include <sstream>
#include "../RequestHandler/RequestHandler.hpp"
#include "ClientState/ClientState.hpp"

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define ORANGE "\033[38;5;214m"
#define RESET "\033[0m"

class ConnectionManager
{
	private:
		std::unordered_map<int, ClientState> m_clients;
		
		std::string distinguishHeadersFromBody(ClientState& clientState , size_t reqNum);
		void appendBodyBytes(ClientState& clientState, const std::string& data);
    	bool hasCompletedRequests(ClientState& clientState);
    	void handleFinishedRequest(ClientState& clientState, size_t index);
		
	public:
		ConnectionManager() = default;
		~ConnectionManager() = default;
		ConnectionManager(const ConnectionManager&) = default;
		ConnectionManager& operator=(const ConnectionManager&) = default;
		ConnectionManager(ConnectionManager&&) noexcept = default;
		ConnectionManager& operator=(ConnectionManager&&) noexcept = default;

		const ParsedRequest& getRequest(int clientId, size_t index = SIZE_MAX) const;
		
		void addClient(int clientId);
		bool processData(int clientId, const std::string& data);
		void resetClientState(int clientId);
		bool clientSentClose(int clientId) const;
		void removeClient(int clientId);
		
		void generateResponseIfReady(int clientId);
		ClientState& getClientStateForTest(int clientId);
		bool accumulateBody(ParsedRequest& request);
		ParsedRequest popFinishedRequest(int clientId);
		bool accumulateChunkedBody(ParsedRequest& request, std::string& data);
		
		std::string processHeaders(ClientState& clientState, size_t reqNum);
		void processLeftover(ClientState& clientState, size_t reqNum, std::string leftover);
	
		//utils
		void printRequest(ClientState& clientState, size_t i);
		void printAllRequests(ClientState& clientState);
		
		bool remainingAfterParsingHeaders(ClientState& clientState);
	
	
	};

#endif
