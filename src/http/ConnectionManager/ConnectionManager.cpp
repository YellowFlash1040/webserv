#include "ConnectionManager.hpp"

// Add a new client
void ConnectionManager::addClient(int clientId)
{
	m_clients.emplace(clientId, ClientState());
}

bool ConnectionManager::clientSentClose(int clientId) const
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return true;

	const ClientRequest& req = it->second.getRequest();
	std::string connHeader = req.getHeader("Connection");
	return (connHeader == "close");
}

void ConnectionManager::resetClientState(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it != m_clients.end())
	{
		it->second.prepareForNextRequest();
	}
}

// Remove a client
void ConnectionManager::removeClient(int clientId)
{
	m_clients.erase(clientId);
}

// Get the response for a state as a string
std::string ConnectionManager::getResponse(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return "";

	ClientState& state = it->second;
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << "ready to send? " << state.isReadyToSend() << "\n";
	if (!state.isReadyToSend())
		return "";

	return state.getRespObj().toString(); // assuming Response has toString()
}

const ClientRequest& ConnectionManager::getRequest(int clientId) const
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("getRequest: clientId not found");

	const ClientRequest& req = it->second.getRequest();
	std::cerr << "getRequest(): method=" << req.getMethod()
			  << " uri=" << req.getUri()
			  << " version=" << req.getHttpVersion() << "\n";
	return req;
}

bool ConnectionManager::processData(int clientId, const std::string& data)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
	{
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
				  << " clientId " << clientId << " not found in m_clients\n";
		return false;
	}

	ClientState& state = it->second;

	appendDataToBuffers(state, data);
	checkAndParseHeaders(state);
	return checkAndProcessBody(state);
}

void ConnectionManager::appendDataToBuffers(ClientState& state, const std::string& data)
{
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
			  << " received " << data.size() << " bytes:\n" << data << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
			  << " before append: headersComplete=" << state.isHeadersComplete()
			  << ", bodyBuffer.size()=" << state.getBodyBuffer().size()
			  << ", headerBuffer.size()=" << state.getHeaderBuffer().size() << "\n";

	if (!state.isHeadersComplete())
	{
		state.appendToHeaderBuffer(data);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
				  << " appended to headerBuffer, size now=" << state.getHeaderBuffer().size() << "\n";
	}
	else
	{
		state.appendToBodyBuffer(data);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
				  << " appended to bodyBuffer, size now=" << state.getBodyBuffer().size() << "\n";
	}
}

void ConnectionManager::checkAndParseHeaders(ClientState& state)
{
	if (!state.isHeadersComplete() && m_parser.bodyComplete(state))
	{
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " headers parsed!\n";
		int contentLength = m_parser.extractContentLength(state);
		state.setContentLength(contentLength);
		state.setHeadersComplete(true);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
				  << " headersComplete=true, contentLength=" << contentLength << "\n";

		// TO DO: chunked handling if needed
		// if (m_parser.isChunked(state))
		// {
		// 	state.setChunked(true);
		// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
		// 	          << " Transfer-Encoding: chunked detected\n";
		// }
	}
}

bool ConnectionManager::checkAndProcessBody(ClientState& state)
{
	if (!state.isHeadersComplete())
		return false;

	bool bodyComplete = m_parser.bodyComplete(state);
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
			  << " headersComplete = true, bodyComplete = " << bodyComplete
			  << ", bodyBuffer.size()=" << state.getBodyBuffer().size() << "\n";

	if (!bodyComplete)
	{
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
				  << " body not complete yet, waiting for more data\n";
		return false;
	}

	// Parse request
	ClientRequest request = m_parser.parseCompleteRequest(state);
	std::cerr << "Parsed request: method=" << request.getMethod()
			  << " uri=" << request.getUri()
			  << " version=" << request.getHttpVersion() << "\n";
	state.setRequest(request);

	// Handle request
	const ServerResponse& respObj = m_handler.handleRequest(request);
	std::string response = respObj.toString();
	state.setResponse(respObj);
	state.setReadyToSend(true);

	// Debug output
	printRequestDebug(state, request);
	printResponseDebug(respObj, response);

	return true; // ready to send
}

void ConnectionManager::printRequestDebug(const ClientState& state, const ClientRequest& request) const
{
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " Request: raw\n"
			  << state.getFullRequestBuffer() << "\n\n";

	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " HTTP Request (parsed):" << RESET << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
			  << " Method: " << RESET << request.getMethod() << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
			  << " URI: " << RESET << request.getUri() << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
			  << " HTTP Version: " << RESET << request.getHttpVersion() << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW << " Request Headers:" << RESET << "\n";
	for (const auto& header : request.getHeaders())
		std::cout << "  " << YELLOW << header.first << ": " << RESET << header.second << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
			  << " Request Body: " << RESET << request.getBody() << "\n\n";
}

void ConnectionManager::printResponseDebug(const ServerResponse& respObj,
	const std::string& response) const
{
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " Response (parsed):" << RESET << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE
			  << " Response Status: " << RESET << respObj.getStatusCode() << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE << " Response Headers:" << RESET << "\n";
	for (const auto& header : respObj.getHeaders())
		std::cout << "  " << header.first << ": " << header.second << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE
			  << " Response Body: " << RESET << respObj.getBody() << "\n\n";

	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " Response(stringified): " << RESET "\n"
			  << response << "\n\n";
}