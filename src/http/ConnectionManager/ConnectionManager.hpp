#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdint>
#include <sstream>
#include "../ParsedRequest/ParsedRequest.hpp"
#include "ClientState/ClientState.hpp"
#include "../utils/ReqContextPrinter/ReqContextPrinter.hpp"
#include "../utils/ParsedRequestUtils/ParsedRequestUtils.hpp"
#include "../config/Config/Config.hpp"

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define ORANGE "\033[38;5;214m"
#define TEAL "\033[36m"
#define BLUE "\033[38;2;100;149;237m"
#define RED "\033[31m"
#define MINT "\033[38;2;150;255;200m"
#define RESET "\033[0m"

class ConnectionManager
{
	private:
		// TODO: make m_config const once Config methods are const-correct
		Config& m_config;
		
		std::unordered_map<int, ClientState> m_clients;
		
	public:
		ConnectionManager() = delete;
		
		// TODO: make m_config const once Config methods are const-correct
		ConnectionManager(Config& config);
		~ConnectionManager() = default;
		ConnectionManager(const ConnectionManager&) = default;
		ConnectionManager& operator=(const ConnectionManager&) = default;
		ConnectionManager(ConnectionManager&&) noexcept = default;
		ConnectionManager& operator=(ConnectionManager&&) noexcept = default;

		const ParsedRequest& getRequest(int clientId, size_t index = SIZE_MAX) const;
		
		void addClient(int clientId);
		bool clientSentClose(int clientId) const;
		void removeClient(int clientId);
		ClientState& getClientStateForTest(int clientId);
		
		bool processData(int clientId, const std::string& tcpData);
		size_t processReqs(int clientId, const std::string& tcpData);
		void genRespsForReadyReqs(int clientId);
		std::string genResp(int clientId);
		
		//for gtests
		ParsedRequest popFinishedReq(int clientId);
	};

#endif
