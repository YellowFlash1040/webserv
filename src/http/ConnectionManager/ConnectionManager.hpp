#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include "../RequestParser/RequestParser.hpp"
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
		RequestParser m_parser;
		RequestHandler m_handler;
		
		void appendDataToBuffers(ClientState& state, const std::string& data);
		void trySeparateHeadersFromBody(ClientState& state);
		bool checkAndProcessBody(ClientState& state);
		void printRequestDebug(const ClientState& state, const ParsedRequest& request) const;
		void printResponseDebug(const ServerResponse& respObj, const std::string& response) const;

	public:
		ConnectionManager() = default;
		~ConnectionManager() = default;
		ConnectionManager(const ConnectionManager&) = default;
		ConnectionManager& operator=(const ConnectionManager&) = default;
		ConnectionManager(ConnectionManager&&) noexcept = default;
		ConnectionManager& operator=(ConnectionManager&&) noexcept = default;

		const ParsedRequest& getRequest(int clientId) const;
		std::string getResponse(int clientId);
		
		void addClient(int clientId);
		bool processData(int clientId, const std::string& data);
		void resetClientState(int clientId);
		bool clientSentClose(int clientId) const;
		void removeClient(int clientId);

		

};

#endif
