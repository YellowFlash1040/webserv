#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "../RawRequest/RawRequest.hpp"
#include "ClientState/ClientState.hpp"
#include "../utils/utils.hpp"
#include "../config/Config/Config.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "../RequestHandler/RequestHandler.hpp"
#include "../Response/FileHandler/FileHandler.hpp"
#include "Client.hpp"
#include "Server.hpp"

#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <filesystem>

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
		const Config& m_config;
		Server& m_server;
		std::unordered_map<Client*, ClientState> m_clients;
		
	public:
		ConnectionManager() = delete;
		
		// TODO: make m_config const once Config methods are const-correct
		ConnectionManager(const Config& config, Server& server);
		~ConnectionManager() = default;
		ConnectionManager(const ConnectionManager&) = default;
		ConnectionManager& operator=(const ConnectionManager&) = delete;
		ConnectionManager(ConnectionManager&&) noexcept = default;
		ConnectionManager& operator=(ConnectionManager&&) noexcept = delete;

		const RawRequest& getRawRequest(Client& client, size_t index = SIZE_MAX) const;

		void addClient(Client& client);
		void removeClient(Client& client);
		ClientState& getClientState(Client& client);
		
		bool processData(Client& client, const std::string& tcpData);
		
		
		void genResps(Client& client);
		size_t processReqs(Client& client, const std::string& tcpData);

		void enqueueInternRedirResp(const std::string& newUri,
                                               RequestContext& ctx,
                                               RequestHandler& reqHandler,
                                               Client& client, 
											   ClientState& clientState);
		
		
		void enqueueExternRedirResp(std::string& newUri, RequestContext newCtx, RequestHandler& reqHandler, Client& client);
	
		
	};

#endif
