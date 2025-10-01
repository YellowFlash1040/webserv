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
	
	const ClientState& clientState = it->second;
	if (clientState.getParsedRequestCount() == 0)
		return false; // no requests yet
		
	const ParsedRequest& req = clientState.getReqObj(clientState.getParsedRequestCount() - 1);
	std::string connHeader = req.getHeader("Connection");
	return (connHeader == "close");
}

// Remove a client
void ConnectionManager::removeClient(int clientId)
{
	m_clients.erase(clientId);
}


const ParsedRequest& ConnectionManager::getRequest(int clientId, size_t index) const
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("getRequest: clientId not found");

	if (index == SIZE_MAX)
		return it->second.getLatestReqObj(); // const getter
	
	return it->second.getReqObj(index); // const getter
}

ClientState& ConnectionManager::getClientStateForTest(int clientId)
{
	return m_clients.at(clientId);
}



bool ConnectionManager::processData(int clientId, const std::string& data)
{
	
	std::cout << "\033[33mDEBUG: processData\033[0m" << std::endl;
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return false;

	ClientState& clientState = it->second;
	printAllRequests(clientState);
	
	// if (clientState.getParsedRequestCount() > 0 && req.isRequestDone())
	if (clientState.getParsedRequestCount() == 0 || clientState.getLatestRequest().isRequestDone())
	{
		std::cout << BLUE << "processData: a new request made" << RESET << std::endl;

		// Add a new ParsedRequest to the client
		clientState.addParsedRequest();
	}
	
	ParsedRequest& req = clientState.getLatestRequest();
	// Append all incoming bytes to _tempBuffer
	req.appendTempBuffer(data);
	
	std::cout <<  "tempBuffer is" << req.getTempBuffer() << "\n";
	
	

	// Determine leftover bytes after headers
	
	//if (req.isHeadersDone() == false)
	{
		remainingAfterParsingHeaders(clientState);
		
		printAllRequests(clientState);
		std::cout << "tempBuffer "<< req.getTempBuffer() << "\n";
		
		if ((req.getTempBuffer().empty() == false && req.isBodyDone() == false)
		|| (req.getBodyType() == BodyType::CHUNKED && req.isTerminatingZero() == false))
		{
			// Not enough bytes yet for a request with a body
			std::cout << BLUE << "processData: not enough bytes yet" RESET << std::endl;
			return false;
		}
	}
	
	if (req.isBodyDone() == false && req.getBodyType() == BodyType::SIZED)
		appendBodyBytes(clientState, req.getTempBuffer());
		
	return hasCompletedRequests(clientState);
}

void ConnectionManager::remainingAfterParsingHeaders(ClientState& clientState)
{
	std::cout << "\033[33mDEBUG: remainingAfterParsingHeaders\033[0m" << std::endl;
	ParsedRequest& request = clientState.getLatestRequest();
	// const std::string& temp = request.getTempBuffer();
	
	printRequest(clientState, 0);
	
	if (request.isHeadersDone() == false)
		separateHeadersFromBody(request);

	if (request.getTempBuffer().empty() == false)
	{
		// Depending on body type, append to correct buffer
		switch (request.getBodyType())
		{
			case BodyType::SIZED:
				{	
					std::cout << "inside the case, tempBuffer over is " << request.getTempBuffer()
					<< " and contentLengthbuffer is " << request.getContentLengthBuffer() << "\n";
					request.appendToContentLengthBuffer(request.getTempBuffer());
						if (request.contentLengthComplete())
						{
							request.setBody(request.getContentLengthBuffer());
							request.setBodyDone();
							request.getTempBuffer().clear(); // all consumed
						}
					std::cout << "inside the case again, left over is " << request.getTempBuffer()
					<< " and contentLengthbuffer is " << request.getContentLengthBuffer() << "\n";
				}
				break;
			case BodyType::CHUNKED:
			{
				request.appendToChunkedBuffer(request.getTempBuffer());
				std::string decoded = request.decodeChunkedBody();
				if (request.isTerminatingZero())
				{
					printf ("found terminating\n");
					request.setBody(decoded);
					request.setBodyDone();
					request.getTempBuffer().clear(); // all consumed
				}
			}
				break;
			case BodyType::NO_BODY:
			case BodyType::ERROR:
				break; // nothing to do
		}
	}
}

void ConnectionManager::separateHeadersFromBody(ParsedRequest& request)
{
    const std::string& temp = request.getTempBuffer();
    size_t headerEnd = temp.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return; // headers incomplete

    std::string headerPart = temp.substr(0, headerEnd + 4);

    try
    {
        request.parseRequestLineAndHeaders(headerPart);
    }
    catch (...)
    {
        request.setBodyType(BodyType::ERROR);
        request.setBodyDone();
        request.clearTempBuffer();
        throw;
    }

    if (request.getBodyType() == BodyType::NO_BODY)
    {
        request.setBodyDone();
        request.setRequestDone();
    }

    // Keep leftover (after headers) in tempBuffer
   	std::string leftover =  temp.substr(headerEnd + 4); // leftover for body
	request.setTempBuffer(leftover);
}



void ConnectionManager::appendBodyBytes(ClientState& clientState, const std::string& data)
{
	std::cout << "\033[33mDEBUG: appendBodyBytes\033[0m" << std::endl;
	ParsedRequest& request = clientState.getLatestRequest();

	std::cout << GREEN << "appendBodyBytes, before appending:" << RESET << "\n"
		<< "ContentLengthBuffer() = " << request.getContentLengthBuffer() << "\n"
		<< "ChunkedBuffer() = " << request.getChunkedBuffer() << "\n";
		
	switch (request.getBodyType())
	{
		case BodyType::SIZED:
		{
			request.appendToContentLengthBuffer(data);
			if (request.contentLengthComplete())
				request.setBodyDone();
			std::cout << GREEN << "appendBodyBytes, after appending:" << RESET << "\n"
			<< "ContentLengthBuffer() = " << request.getContentLengthBuffer() << "\n";
			break;
		}

		case BodyType::CHUNKED:
		{
			request.appendToChunkedBuffer(data);
			std::string decoded = request.decodeChunkedBody();
			request.setBody(decoded);
			if (request.isTerminatingZero())
				request.setBodyDone();
			std::cout << GREEN << "appendBodyBytes, after appending:" << RESET << "\n"
			<< "ChunkedBuffer() = " << request.getChunkedBuffer() << "\n";
			break;
		}

		case BodyType::NO_BODY:
			// Nothing to append
			break;

		case BodyType::ERROR:
			throw std::runtime_error("Cannot append body data: request in ERROR state");
	}
	
}


//returns true if one or more requests were processed
bool ConnectionManager::hasCompletedRequests(ClientState& clientState)
{
	std::cout << "\033[33mDEBUG: hasCompletedRequests\033[0m" << std::endl;

	bool anyRequestDone = false;

	for (size_t i = 0; i < clientState.getParsedRequestCount(); ++i)
	{
		ParsedRequest& req = clientState.getParsedRequest(i);
		
		// Wait until the request body is comple
		if (!req.isBodyDone())
			break;
			
		//Handle finished request (send response, log, etc.)
		// Here we assume handleFinishedRequest manages sending and cleanup
		handleFinishedRequest(clientState, i);

		anyRequestDone = true;

		if (!req.isRequestDone())
		{
			std::cout << RED << "hasCompletedRequests: no more requests done" << RESET << "\n";
			break; // cannot process further requests until this one is done
		}

        // Optional: if you remove finished requests from the list
        // clientState.popFirstFinishedRequest();
        // --i; // adjust index since we removed a request
	}
	std::cout << RED << "hasCompletedRequests: found requests done\n" << RESET << "\n";
	return anyRequestDone;
}

void ConnectionManager::handleFinishedRequest(ClientState& clientState, size_t index)
{
	std::cout << "\033[33mDEBUG: handleFinishedRequest\033[0m" << std::endl;
    ParsedRequest& finishedReq = clientState.getParsedRequest(index);

    // Send response / logging / etc.
    // sendResponse(req);

    // Determine if there is leftover data in body buffers
    std::string leftover;

    if (finishedReq.getBodyType() == BodyType::CHUNKED)
        leftover = finishedReq.getChunkedBuffer();
    else if (finishedReq.getBodyType() == BodyType::SIZED)
        leftover = finishedReq.getContentLengthBuffer();

    // Clear finished request's buffers
    finishedReq.clearBodyBuffer();
    finishedReq.clearChunkedBuffer();
    finishedReq.clearContentLengthBuffer();
	
	printf ("handleFinishedRequest: before addParsedRequest\n");
	printAllRequests(clientState);
	
    // If leftover exists, move it to a new request
	std::cout << "\033[31mDEBUG: handleFinishedRequest:leftover is\033[0m " << leftover << std::endl;
    if (!leftover.empty())
    {
		std::cout << "\033[31mDEBUG: handleFinishedRequest: a new request made\033[0m" << std::endl;
        ParsedRequest& newReq = clientState.addParsedRequest();
        newReq.appendTempBuffer(leftover);
    }
	printf ("handleFinishedRequest: after addParsedRequest\n");
	printAllRequests(clientState);
	printf ("\n");
	
	// Mark request done
    finishedReq.setRequestDone();
}

std::string ConnectionManager::processHeaders(ClientState& clientState, size_t reqNum)
{
	printf ("in processHeaders:\n");
	printAllRequests(clientState);
	
	ParsedRequest& request = clientState.getParsedRequest(reqNum);
	const std::string& buf = request.getRlAndHeadersBuffer();

	std::cout << GREEN << "DEBUG" << RESET
		<< "[processHeaders]: req #" << reqNum
		<< " Checking rlAndHeadersBuffer of size " << buf.size() << "\n";

	size_t pos = buf.find("\r\n\r\n");
	if (pos == std::string::npos)
	{
		std::cout << GREEN << "DEBUG" << RESET
				  << "[processHeaders]: req #" << reqNum
				  << " CRLFCRLF not found yet, waiting for more data.\n";
		return ""; // waiting for more data
	}

	// Split buffer into header part and leftover
	std::string rlAndHeaderPart = buf.substr(0, pos + 4);
	std::string leftover = buf.substr(pos + 4);

	std::cout << GREEN << "DEBUG" << RESET
			  << "[processHeaders]: req #" << reqNum
			  << " Headers complete. Header size = " << rlAndHeaderPart.size()
			  << ", leftover size = " << leftover.size() << "\n";
	
	// Mark headers done
	request.setHeadersDone();
	
	try
	{
		request.parseRequestLineAndHeaders(rlAndHeaderPart);
	}
	catch(const std::exception& e)
	{
		request.setBodyType(BodyType::ERROR);
		request.setBodyDone(); // mark as done so it doesn't hang
		request.clearRlAndHeadersBuffer();
		throw std::runtime_error("Header parse error: " + std::string(e.what()));
	}
	
	std::cout << GREEN << "DEBUG" << RESET
		<< "[processHeaders]: req #" << reqNum
		<< " Parsed headers: Method=" << request.getMethod()
		<< ", URI=" << request.getUri()
		<< ", Version=" << request.getHttpVersion()
		<< ", contentLength=" << request.getContentLength() << "\n";
	
	// If no body expected
	if (request.getBodyType() == BodyType::NO_BODY)
	{
		request.setBodyDone();
		request.clearRlAndHeadersBuffer();
		if (!leftover.empty())
		{
			clientState.prepareNextRequestWithLeftover(leftover);
			// do not clear leftover, let the next function handle it
		}
	}
	return leftover;
}

ParsedRequest ConnectionManager::popFinishedRequest(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("Client not found");

	return it->second.popFirstFinishedRequest();
}

// bool ConnectionManager::accumulateChunkedBody(ParsedRequest& request, std::string& data)
// {
// 	std::string& buffer = request.getChunkedBuffer(); // accumulate here
// 	size_t pos = 0;

// 	while (pos < data.size())
// 	{
// 		size_t crlfPos = data.find("\r\n", pos);
// 		if (crlfPos == std::string::npos)
// 			break;

// 		std::string sizeLine = data.substr(pos, crlfPos - pos);
// 		size_t chunkSize = 0;

// 		try
// 		{
// 			chunkSize = std::stoul(sizeLine, nullptr, 16);
// 		}
// 		catch (...)
// 		{
// 			throw std::runtime_error("Invalid chunk size: " + sizeLine);
// 		}

// 		pos = crlfPos + 2; // move past CRLF

// 		if (pos + chunkSize > data.size())
// 			break; // incomplete chunk

// 		// Append chunk to chunked buffer
// 		buffer.append(data.substr(pos, chunkSize));
// 		pos += chunkSize;
		
// 		// Expect CRLF after chunk data
// 		if (pos + 2 > data.size())
// 			break;
// 		if (data.substr(pos, 2) != "\r\n")
// 			throw std::runtime_error("Missing CRLF after chunk");
// 		pos += 2;
		
// 		// Last chunk?
// 		if (chunkSize == 0)
// 		{
// 			request.setBody(buffer); // finalize body from chunked buffer
// 			request.setBodyDone();
// 			request.clearChunkedBuffer(); //clear raw chunked data
// 			data = (pos < data.size()) ? data.substr(pos) : "";
// 			return true;
// 		}
// 	}
	
// 	 // remove processed bytes
// 	data = data.substr(pos);
// 	return request.isBodyDone();
// }




static std::string bodyTypeToString(BodyType t)
{
	switch (t)
	{
		case BodyType::NO_BODY:    return "NO_BODY";
		case BodyType::SIZED:   return "SIZED";
		case BodyType::CHUNKED: return "CHUNKED";
		default:                return "UNKNOWN";
	}
}

// Print a single ParsedRequest at index `i`
void ConnectionManager::printRequest(ClientState& clientState, size_t i)
{
	if (i >= clientState.getParsedRequestCount())
	{
		std::cout << "Index " << i << " out of range, total requests: "
				  << clientState.getParsedRequestCount() << "\n";
		return;
	}

	const ParsedRequest& req = clientState.getParsedRequest(i);
	std::cout << TEAL << "Request #" << i << RESET << "\n"
			  << "Method = " << req.getMethod()
			  << ", URI = " << req.getUri()
			  << ", BodyType = " << bodyTypeToString(req.getBodyType())
			  << ", HeadersDone = " << (req.isHeadersDone() ? "true" : "false")
			  << ", BodyDone = " << (req.isBodyDone() ? "true" : "false")
			  << ", RequestDone = " << (req.isRequestDone() ? "true" : "false")
			  << "\n";
			  if (!req.getBody().empty())
				std::cout << "  Body: '" << req.getBody() << "'\n";

	// Print key headers if available
	std::string host = req.getHeader("Host");
	std::string te   = req.getHeader("Transfer-Encoding");
	std::string cl   = req.getHeader("Content-Length");

	if (!host.empty())
		std::cout << "  Host: " << host << "\n";
	if (!te.empty())
		std::cout << "  Transfer-Encoding: " << te << "\n";
	if (!cl.empty())
		std::cout << "  Content-Length: " << cl << "\n";
}

// Print all requests in the ClientState
void ConnectionManager::printAllRequests(ClientState& clientState)
{
	size_t count = clientState.getParsedRequestCount();
	if (count == 0)
	{
		std::cout << "No requests in client state.\n";
		return;
	}

	for (size_t i = 0; i < count; ++i)
	{
		printRequest(clientState, i);
	}
	std::cout << "\n";
}