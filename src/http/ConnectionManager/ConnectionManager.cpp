#include "ConnectionManager.hpp"

// Add a new client
void ConnectionManager::addClient(int clientId)
{
	m_clients.emplace(clientId, ClientState());
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

// Process incoming data for a client
bool ConnectionManager::processData(int clientId, const std::string& data)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
	{
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " clientId " << clientId << " not found in m_clients\n";
		return false;
	}

	ClientState& state = it->second;
	
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " received " << data.size() << " bytes:\n" << data << "\n";
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " before append: headersComplete=" << state.isHeadersComplete() 
			  << ", bodyBuffer.size()=" << state.getBodyBuffer().size() 
			  << ", headerBuffer.size()=" << state.getHeaderBuffer().size() << "\n";
			  
	// Append incoming data
	if (!state.isHeadersComplete())
	{
		state.appendToHeaderBuffer(data);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " appended to headerBuffer, size now=" << state.getHeaderBuffer().size() << "\n";
	}
	else
	{
		state.appendToBodyBuffer(data);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " appended to bodyBuffer, size now=" << state.getBodyBuffer().size() << "\n";
	}

	// Check headers
	if (!state.isHeadersComplete() && m_parser.bodyComplete(state))
	{
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " headers parsed!\n";
		int contentLength = m_parser.extractContentLength(state);
		state.setContentLength(contentLength);
		state.setHeadersComplete(true);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " headersComplete=true, contentLength=" << contentLength << "\n";
		
		// TO DO: chunked handling if needed
		// if (m_parser.isChunked(state))
		// {
		// 	state.setChunked(true);
		// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " Transfer-Encoding: chunked detected\n";
		// }
	}

	// Check body
	if (state.isHeadersComplete())
	{
		bool bodyComplete = m_parser.bodyComplete(state);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " headersComplete = true, bodyComplete = " << bodyComplete 
				  << ", bodyBuffer.size()=" << state.getBodyBuffer().size() << "\n";
		
		if (bodyComplete)
		{
			Request request = m_parser.parseCompleteRequest(state);
			
			const Response& respObj = m_handler.handleRequest(request);
			std::string response = respObj.toString();
			state.setResponse(respObj);
			state.setReadyToSend(true); 
			
			// for debugging:
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " Request: raw\n"
					<< state.getFullRequestBuffer() << "\n\n";

			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " HTTP Request (parsed):" << RESET << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW << " Method: " << RESET << request.getMethod() << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW << " URI: " << RESET << request.getUri() << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW << " HTTP Version: " << RESET << request.getHttpVersion() << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW << " Request Headers:" << RESET << "\n";
			for (const auto& header : request.getHeaders())
				std::cout << "  " << YELLOW << header.first << ": " << RESET << header.second << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW << " Request Body: " << RESET << request.getBody() << "\n\n";

			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " Response (parsed):" << RESET << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE << " Response Status: " << RESET << respObj.getStatusCode() << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE << " Response Headers:" << RESET << "\n";
			for (const auto& header : respObj.getHeaders())
				std::cout << "  " << header.first << ": " << header.second << "\n";
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE << " Response Body: " << RESET << respObj.getBody() << "\n\n";

			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " Response(stringified): " << RESET "\n"
					<< response << "\n\n";
			
			// // Decide whether to keep the client
			// std::string connHeader = request.getHeader("Connection");
			// bool clientSentClose = (connHeader == "close");

			// if (clientSentClose)
			// {
			// 	removeClient(clientId);  // only remove if client wants to close
			// }
			// else
			// {
			// 	state.prepareForNextRequest();  // keep alive
			// }

			return true; // ready to send
		}
		else
		{
			std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " body not complete yet, waiting for more data\n";
		}
	}

	return false;
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

const Request& ConnectionManager::getRequest(int clientId) const
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("getRequest: clientId not found");

	return it->second.getRequest(); // assuming ClientState has getRequest()
}