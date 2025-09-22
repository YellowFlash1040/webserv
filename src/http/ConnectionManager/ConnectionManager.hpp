#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <map>
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
		std::map<int, ClientState> m_clients;
		RequestParser m_parser;
		RequestHandler m_handler;

	public:
		ConnectionManager() = default;
		~ConnectionManager() = default;
		ConnectionManager(const ConnectionManager&) = default;
		ConnectionManager& operator=(const ConnectionManager&) = default;
		ConnectionManager(ConnectionManager&&) noexcept = default;
		ConnectionManager& operator=(ConnectionManager&&) noexcept = default;

		void addClient(int clientId);
		bool processData(int clientId, const std::string& data);
		void resetClientState(int clientId);
		void removeClient(int clientId);
		
		std::string getResponse(int clientId);
		const Request& getRequest(int clientId) const;
};

#endif
