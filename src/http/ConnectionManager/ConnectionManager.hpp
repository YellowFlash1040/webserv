#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <filesystem>

#include "../RawRequest/RawRequest.hpp"
#include "ClientState/ClientState.hpp"
#include "../utils/utils.hpp"
#include "../config/Config/Config.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "../../RequestHandler/RequestHandler.hpp"
#include "../FileUtils/FileUtils.hpp"
#include "debug.hpp"

class ConnectionManager
{
	private:
		const Config& m_config;
		
		std::unordered_map<int, ClientState> m_clients;
		
		void genResps(int clientId, const NetworkEndpoint& endpoint);
		void printAllResponses(const ClientState& clientState);
		
		
	public:
		ConnectionManager() = delete;
		ConnectionManager(const Config& config);
		~ConnectionManager() = default;
		ConnectionManager(const ConnectionManager&) = default;
		ConnectionManager& operator=(const ConnectionManager&) = delete;
		ConnectionManager(ConnectionManager&&) noexcept = default;
		ConnectionManager& operator=(ConnectionManager&&) noexcept = delete;

		void addClient(int clientId);
		void removeClient(int clientId);
		ClientState& getClientState(int clientId);
		bool processData(const NetworkEndpoint& endpoint, int clientId, const std::string& tcpData);
		

		
		size_t processReqs(int clientId, const std::string& tcpData); //actually private
	
	};

#endif
