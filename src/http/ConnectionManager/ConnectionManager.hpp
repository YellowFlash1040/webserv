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
#define TEAL "\033[36m"
#define BLUE "\033[38;2;100;149;237m"
#define RED "\033[31m"
#define RESET "\033[0m"

class ConnectionManager
{
	private:
		std::unordered_map<int, ClientState> m_clients;
		
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
		
		bool processData(int clientId, const std::string& tcpData);
		size_t processReqs(int clientId, const std::string& tcpData);
		void genRespsForReadyReqs(int clientId);
		std::string genResp(int clientId);
		
		
		bool clientSentClose(int clientId) const;
		void removeClient(int clientId);
		
		
		ClientState& getClientStateForTest(int clientId);
		ParsedRequest popFinishedReq(int clientId);
		
		
		std::string processHeaders(ClientState& clientState, size_t reqNum);
		
	
		//utils
		void printRequest(ClientState& clientState, size_t i);
		void printAllRequests(ClientState& clientState);
		
		// void remainingAfterParsingHeaders(ClientState& clientState);
		void separateHeadersFromBody(ParsedRequest& request);
	
		std::string bodyTypeToString(BodyType t);
		void printSingleRequest(const ParsedRequest& req, size_t i = 0);
		
		void printBodyBuffers(ParsedRequest& req);
		
		//for gtests
		ParsedRequest popFinishedRequest(int clientId);
	};

#endif
