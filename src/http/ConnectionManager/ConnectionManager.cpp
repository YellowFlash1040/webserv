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

	const ParsedRequest& req = it->second.getRequest();
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

// Get the response for a clientState as a string
std::string ConnectionManager::getResponse(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return "";

	ClientState& clientState = it->second;
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << "ready to send? " << clientState.isReadyToSend() << "\n";
	if (!clientState.isReadyToSend())
		return "";

	return clientState.getRespObj().toString(); // assuming Response has toString()
}

const ParsedRequest& ConnectionManager::getRequest(int clientId) const
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("getRequest: clientId not found");

	const ParsedRequest& req = it->second.getRequest();
	std::cerr << "getRequest(): method=" << req.getMethod()
			  << " uri=" << req.getUri()
			  << " version=" << req.getHttpVersion() << "\n";
	return req;
}

// bool ConnectionManager::processData(int clientId, const std::string& data)
// {
// 	auto it = m_clients.find(clientId);
// 	if (it == m_clients.end())
// 	{
// 		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
// 			<< " clientId " << clientId << " not found in m_clients\n";
// 		return false;
// 	}

// 	ClientState& clientState = it->second;

// 	appendDataToBuffers(clientState, data);
// 	separateHeadersFromBody(clientState);
// 	bool requestIsReady = checkAndProcessBody(clientState);

// 	if (requestIsReady)
// 		resetClientState(clientId); //request fully processed

// 	return requestIsReady;
// }

// void ConnectionManager::appendDataToBuffers(ClientState& clientState, const std::string& data)
// {
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
// 			  << " received " << data.size() << " bytes:\n" << data << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
// 			  << " before append: headersComplete=" << clientState.isHeadersDone()
// 			  << ", bodyBuffer.size()=" << clientState.getBodyBuffer().size()
// 			  << ", headerBuffer.size()=" << clientState.getRlAndHeaderBuffer().size() << "\n";

// 	if (!clientState.isHeadersDone())
// 	{
// 		clientState.appendToRlAndHeaderBuffer(data);
// 		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
// 				  << " appended to Request line and header Buffer, size now = " << clientState.getRlAndHeaderBuffer().size() << "\n";
// 	}
// 	else
// 	{
// 		clientState.appendToBodyBuffer(data);
// 		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
// 				  << " appended to body Buffer, size now = " << clientState.getBodyBuffer().size() << "\n";
// 	}
// }

// void ConnectionManager::separateHeadersFromBody(ClientState& clientState)
// {
// 	if (clientState.hasHeadersBeenSeparatedFromBody())
// 	{
// 		std::cout << YELLOW << "DEBUG[HEADERS]: " << RESET
// 				  << "Already marked complete, skipping.\n";
// 		return;
// 	}
	
// 	const std::string& buf = clientState.getRlAndHeaderBuffer();

// 	std::cout << YELLOW << "DEBUG[HEADERS]: " << RESET
// 			  << "headerBuffer.size()=" << buf.size()
// 			  << " content='" << buf << "'\n";

// 	// Look for the CRLFCRLF marker that terminates headers
// 	size_t pos = buf.find("\r\n\r\n");
// 	if (pos == std::string::npos)
// 	{
// 		std::cout << YELLOW << "DEBUG[HEADERS]: " << RESET
// 				  << "CRLFCRLF not found yet, waiting for more data.\n";
// 		return; // no complete headers yet, wait for more data
// 	}

// 	std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
// 			  << "CRLFCRLF found at position " << pos << ".\n";

// 	// Split header and body
// 	std::string headerPart = buf.substr(0, pos + 4); // include CRLFCRLF
// 	std::string bodyPart   = buf.substr(pos + 4);

// 	std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
// 			  << "headerPart.size()=" << headerPart.size()
// 			  << ", bodyOverflow.size()=" << bodyPart.size() << "\n";

// 	// Replace the header buffer with only the header part
// 	// and move overflow into body buffer
// 	clientState.prepareForNextRequest();
// 	clientState.appendToRlAndHeaderBuffer(headerPart);
// 	if (!bodyPart.empty())
// 	{
// 		clientState.appendToBodyBuffer(bodyPart);
// 		std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
// 				  << "Moved " << bodyPart.size()
// 				  << " bytes into bodyBuffer (overflow after headers).\n";
// 	}

// 	// Now mark headers complete
// 	clientState.setHeadersDone(true);

// 	// Extract Content-Length (or Transfer-Encoding) from headers
// 	int contentLength = m_parser.extractContentLength(clientState);
// 	clientState.setContentLength(contentLength);

// 	std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
// 			  << "headersComplete=true, contentLength=" << contentLength
// 			  << ", bodyBuffer.size()=" << clientState.getBodyBuffer().size() << "\n";

// 	// TODO: if chunked, set clientState.setChunked(true);
// }


// bool ConnectionManager::checkAndProcessBody(ClientState& clientState)
// {
// 	if (!clientState.isHeadersDone())
// 		return false;

// 	bool bodyDone = m_parser.bodyDone(clientState);
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
// 			  << " headersComplete = true, bodyDone = " << bodyDone
// 			  << ", bodyBuffer.size()=" << clientState.getBodyBuffer().size() << "\n";

// 	if (!bodyDone)
// 	{
// 		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
// 				  << " body not complete yet, waiting for more data\n";
// 		return false;
// 	}

// 	// Parse request
// 	ParsedRequest request = m_parser.parseBufferedRequest(clientState);
// 	std::cout << "Parsed request: method=" << request.getMethod()
// 		<< " uri=" << request.getUri()
// 		<< " version=" << request.getHttpVersion() << "\n";
// 	clientState.setRequest(request);

// 	// Handle request
// 	const ServerResponse& respObj = m_handler.handleRequest(request);
// 	std::string response = respObj.toString();
// 	clientState.setResponse(respObj);
// 	clientState.setReadyToSend(true);

// 	// Debug output
// 	printRequestDebug(clientState, request);
// 	printResponseDebug(respObj, response);

// 	return true; // ready to send
// }

// void ConnectionManager::printRequestDebug(const ClientState& clientState, const ParsedRequest& request) const
// {
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " Request: raw\n"
// 			  << clientState.getFullRequestBuffer() << "\n\n";

// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " HTTP Request (parsed):" << RESET << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
// 			  << " Method: " << RESET << request.getMethod() << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
// 			  << " URI: " << RESET << request.getUri() << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
// 			  << " HTTP Version: " << RESET << request.getHttpVersion() << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW << " Request Headers:" << RESET << "\n";
// 	for (const auto& header : request.getHeaders())
// 		std::cout << "  " << YELLOW << header.first << ": " << RESET << header.second << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << YELLOW
// 			  << " Request Body: " << RESET << request.getBody() << "\n\n";
// }

// void ConnectionManager::printResponseDebug(const ServerResponse& respObj,
// 	const std::string& response) const
// {
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " Response (parsed):" << RESET << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE
// 			  << " Response Status: " << RESET << respObj.getStatusCode() << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE << " Response Headers:" << RESET << "\n";
// 	for (const auto& header : respObj.getHeaders())
// 		std::cout << "  " << header.first << ": " << header.second << "\n";
// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << ORANGE
// 			  << " Response Body: " << RESET << respObj.getBody() << "\n\n";

// 	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << " Response(stringified): " << RESET "\n"
// 			  << response << "\n\n";
// }





















bool ConnectionManager::processData(int clientId, const std::string& data)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
	{
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
			<< " clientId " << clientId << " not found in m_clients\n";
		return false;
	}

	ClientState& clientState = it->second;

	if (clientState.isHeadersDone())
	{
		clientState.appendToBodyBuffer(data);
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
				  << " appended to body Buffer, size now = " << clientState.getBodyBuffer().size() << "\n";
	}
	else
	{
		// append incoming bytes to header buffer (request line + headers)
		clientState.appendToRlAndHeaderBuffer(data);
		trySeparateHeadersFromBody(clientState); //may mark headers done & move overflow
	}
	
	// check if we have a full request (headers+body), parser will decide based on Content-Length or chunked
	bool requestIsReady = checkAndProcessBody(clientState);
	if (requestIsReady)
	{
		// To Do: handle leftover bytes for pipelined requests, use a special reset that preserves leftovers.
		clientState.prepareForNextRequestBuffersOnly(); //for gtests
	}

	return requestIsReady;
}


void ConnectionManager::trySeparateHeadersFromBody(ClientState& clientState)
{	
	const std::string& buf = clientState.getRlAndHeaderBuffer();

	std::cout << YELLOW << "DEBUG[HEADERS]: " << RESET
			  << "headerBuffer.size()=" << buf.size()
			  << " content='" << buf << "'\n";

	// Look for the CRLFCRLF marker that terminates headers
	size_t pos = buf.find("\r\n\r\n");
	if (pos == std::string::npos)
	{
		std::cout << YELLOW << "DEBUG[HEADERS]: " << RESET
				  << "CRLFCRLF not found yet, waiting for more data.\n";
		return; // no complete headers yet, wait for more data
	}

	std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
			  << "CRLFCRLF found at position " << pos << ".\n";

	// Split header and body (overflow)
	std::string headerPart = buf.substr(0, pos + 4); // include CRLFCRLF
	std::string bodyPart   = buf.substr(pos + 4);

	std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
			  << "headerPart.size()=" << headerPart.size()
			  << ", bodyOverflow.size()=" << bodyPart.size() << "\n";

	// Replace the header buffer with only the header part
	// and move overflow into body buffer
	clientState.clearRlAndHeaderBuffer();
	clientState.appendToRlAndHeaderBuffer(headerPart);
	if (!bodyPart.empty())
	{
		clientState.appendToBodyBuffer(bodyPart);
		std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
				  << "Moved " << bodyPart.size()
				  << " bytes into bodyBuffer (overflow after headers).\n";
	}

	// Mark headers complete
	clientState.setHeadersDone();

	// Extract Content-Length (or Transfer-Encoding) from headers
	int contentLength = m_parser.extractContentLength(clientState);
	clientState.setContentLength(contentLength);

	std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
			  << "headersComplete=true, contentLength=" << contentLength
			  << ", bodyBuffer.size()=" << clientState.getBodyBuffer().size() << "\n";

	// Check for Transfer-Encoding: chunked
	const std::string& headers = clientState.getRlAndHeaderBuffer();
	if (headers.find("Transfer-Encoding: chunked") != std::string::npos)
    {
		clientState.setChunked(true);
		std::cout<< "Detected Transfer-Encoding: chunked, enabling chunked mode.\n";
	}
	else
	{
		clientState.setChunked(false);
	}
 	std::cout << GREEN << "DEBUG[HEADERS]: " << RESET
			  << "headersComplete=true, contentLength=" << contentLength
			  << ", bodyBuffer.size()=" << clientState.getBodyBuffer().size() << "\n";
}

bool ConnectionManager::checkAndProcessBody(ClientState& clientState)
{

	// Must have headers done first
	if (!clientState.isHeadersDone())
		return false;
	
	// Ask parser whether the body is fully received (content-length or chunked)
	bool bodyDone = m_parser.isBodyDone(clientState);
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
			  << " headersComplete = true, bodyDone = " << bodyDone
			  << ", bodyBuffer.size()=" << clientState.getBodyBuffer().size() << "\n";

	if (!bodyDone)
	{
		std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET
				  << " body not complete yet, waiting for more data\n";
		return false;
	}
	
	// Mark body done
	clientState.setBodyDone();
	
	// Parse full request from buffers (request line+headers already in rl buffer; body in bodyBuffer)
	ParsedRequest request = m_parser.parseBufferedRequest(clientState);
	std::cout << "Parsed request: method=" << request.getMethod()
		<< " uri=" << request.getUri()
		<< " version=" << request.getHttpVersion() << "\n";
	clientState.setRequest(request);

	// Handle request
	const ServerResponse& respObj = m_handler.handleRequest(request);
	std::string response = respObj.toString();
	clientState.setResponse(respObj);
	clientState.setReadyToSend(true);

	// Debug output
	printRequestDebug(clientState, request);
	printResponseDebug(respObj, response);

	return true; // ready to send
}

void ConnectionManager::printRequestDebug(const ClientState& clientState, const ParsedRequest& request) const
{
	std::cout << GREEN << "DEBUG[CONNECTION_MANAGER]:" << RESET << " Request: raw\n"
			  << clientState.getFullRequestBuffer() << "\n\n";

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

