#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "../RawRequest/RawRequest.hpp"
#include "ClientState/ClientState.hpp"
#include "../utils/utils.hpp"
#include "../config/Config/Config.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "../RequestHandler/RequestHandler.hpp"
#include "../FileUtils/FileUtils.hpp"
#include "debug.hpp"

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
		
		std::unordered_map<int, ClientState> m_clients;
		
	public:
		ConnectionManager() = delete;
		
		// TODO: make m_config const once Config methods are const-correct
		ConnectionManager(const Config& config);
		~ConnectionManager() = default;
		ConnectionManager(const ConnectionManager&) = default;
		ConnectionManager& operator=(const ConnectionManager&) = delete;
		ConnectionManager(ConnectionManager&&) noexcept = default;
		ConnectionManager& operator=(ConnectionManager&&) noexcept = delete;

		const RawRequest& getRawRequest(int clientId, size_t index = SIZE_MAX) const;
		
		void addClient(int clientId);
		// bool clientSentClose(int clientId) const;
		void removeClient(int clientId);
		ClientState& getClientState(int clientId);
		
		bool processData(const NetworkEndpoint& endpoint, int clientId, const std::string& tcpData);
		
		void genResps(int clientId, const NetworkEndpoint& endpoint);

		size_t processReqs(int clientId, const std::string& tcpData);
				
		void enqueueInternRedirResp(const std::string& newUri,
                                               const RequestContext& ctx,
                                               RequestHandler& reqHandler,
                                               const NetworkEndpoint& endpoint, 
											   ClientState& clientState, bool shouldClose);
		
		
		void enqueueExternRedirResp(HttpMethod method, const RequestContext& newCtx, RequestHandler& reqHandler,
									const NetworkEndpoint& endpoint, bool shouldClose);
	
		void printAllResponses(const ClientState& clientState);
		
	};

#endif
