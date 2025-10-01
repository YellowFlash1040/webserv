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


















// bool ConnectionManager::processData(int clientId, const std::string& data)
// {
// 	std::cout << GREEN "DEBUG" << RESET 
// 			  << "[processData]: Received " << data.size() << " bytes for client " << clientId << "\n";

// 	auto it = m_clients.find(clientId);
// 	if (it == m_clients.end())
// 	{
// 		std::cout << GREEN "DEBUG" << RESET 
// 				  << "[processData]: clientId " << clientId << " not found\n";
// 		return false;
// 	}

// 	ClientState& clientState = it->second;
	
// 	// If last request is fully done, prepare new request slot
// 	if (clientState.getParsedRequestCount() > 0 &&
// 		clientState.getLatestRequest().isBodyDone())
// 	{
// 		clientState.prepareForNextRequest();
// 		std::cout << GREEN "DEBUG" << RESET
// 				  << "[processData]: Last request was complete, started a new request slot.\n";
// 	}

	
// 	ParsedRequest& latestRequest = clientState.getLatestRequest();
// 	if (!latestRequest.isChunked() &&  latestRequest.getContentLength() == 0)
// 	{
// 		std::cout << GREEN "DEBUG" << RESET 
// 				<< "[processData]: Appending data to latest request buffer (before size: "
// 				<< latestRequest.getRlAndHeadersBuffer().size() << ")\n";

// 		latestRequest.appendToRlAndHeaderBuffer(data);

// 		std::cout << GREEN "DEBUG" << RESET 
// 				<< "[processData]: Latest request buffer size now: "
// 				<< latestRequest.getRlAndHeadersBuffer().size() << "\n";
// 	}
	
// 	else if (!latestRequest.isChunked() && latestRequest.getContentLength() > 0)
// 	{
// 		latestRequest.appendToContentLengthBuffer(data);
// 		std::cout << GREEN << "DEBUG" << RESET
// 			<< "[processData]: Appended " << data.size()
// 			<< " bytes to _contentLengthBuffer, current size: "
// 			<< latestRequest.getContentLengthBuffer().size() << "\n";
		
// 		 if (latestRequest.contentLengthComplete())
// 		{
// 			latestRequest.setBody(latestRequest.getContentLengthBuffer());
// 			latestRequest.setBodyDone();
// 			latestRequest.clearContentLengthBuffer(); // optional cleanup
// 		}
	
// 	}
	
// 	else
// 	{
// 		std::cout << GREEN "DEBUG" << RESET
// 				<< "[processData]: Appending data to chunked buffer (before size: "
// 				<< latestRequest.getChunkedBuffer().size() << ")\n";

// 		latestRequest.appendToChunkedBuffer(data);

// 		std::cout << GREEN "DEBUG" << RESET
// 		  << "[processData]: _chunkedBuffer after append, size=" 
// 		  << latestRequest.getChunkedBuffer().size() 
// 		  << ", contents: '" << latestRequest.getChunkedBuffer() << "'\n";
		
// 		std::cout << GREEN "DEBUG" << RESET
// 				<< "[processData]: Chunked buffer size now: "
// 				<< latestRequest.getChunkedBuffer().size() << "\n";
				
// 		if (latestRequest.terminatingZeroChunkReceived())
// 		{
// 			std::string decoded = latestRequest.decodeChunkedBody();
// 			std::cout << GREEN "DEBUG" << RESET
// 			  << "[processData]: Decoded chunked body: '" << decoded << "'\n";
// 			latestRequest.setBody(decoded);
			
// 			 std::cout << GREEN "DEBUG" << RESET
// 			  << "[processData]: After decoding chunked body, _body size: "
// 			  << latestRequest.getBody().size() 
// 			  << ", contents: '" << latestRequest.getBody() << "'\n";
			
// 			latestRequest.setBodyDone();
// 			latestRequest.clearChunkedBuffer();
// 		}
// 	}

// 	bool anyRequestDone = false;

// 	//Iterate over all requests in order
// 	for (size_t i = 0; i < clientState.getParsedRequestCount(); ++i)
// 	{
// 		ParsedRequest& req = clientState.getParsedRequest(i);
// 		std::string leftover;

// 		// Step 1: Extract headers if not done
// 		if (!req.isHeadersDone())
// 		{
// 			leftover = distinguishHeadersFromBody(clientState, i);
// 			std::cout << GREEN "DEBUG" << RESET 
// 					  << "[processData]: Request #" << i
// 					  << " leftover size: " << leftover.size() << "\n";

// 			// CASE A: leftover belongs to current body, accumulate it
// 			if (!leftover.empty() && (!req.bodyComplete() || (req.isChunked() && !req.terminatingZeroChunkReceived())))
// 			{	
// 				if (req.isChunked())
// 				{
// 					std::cout << GREEN "DEBUG" << RESET
// 			  		<< "[processData]: Skipping leftover accumulation for chunked request ("
// 			  		<< leftover.size() << " bytes)\n";
// 				}
// 				else
// 				{
// 					accumulateBody(req, leftover);
// 					std::cout << GREEN "DEBUG" << RESET
// 						<< "[processData]: Body buffer size now: " 
// 						<< req.getBodyBuffer().size() 
// 						<< ", contents: '" << req.getBodyBuffer() << "'\n";
// 				}
// 				leftover.clear();
// 			}
// 		}

// 		// Step 2: Stop if request not complete
// 		if (!req.isBodyDone())
// 		{
// 			std::cout << GREEN "DEBUG" << RESET 
// 					  << "[processData]: Request #" << i << " not complete yet, waiting for more data.\n";
// 			break; // cannot process further requests until this one is done
// 		}
		
// 		// Step 3: fully done - pop the request and enqueue response
		
// 		// ParsedRequest finished = clientState.popFirstFinishedRequest(); //for production		
		
// 		ParsedRequest finished = req; //for gtests
// 		ServerResponse resp(finished);
// 		clientState.enqueueResponse(resp);
// 		clientState.setReadyToSend(true);
// 		anyRequestDone = true;
// 		std::cout << GREEN "DEBUG" << RESET 
// 				  << "[processData]: Request complete, response enqueued for URI: "
// 				  << finished.getUri() << "\n";
		
// 		// Step 4: Handle leftover bytes for next request
// 		if (!leftover.empty())
// 		{
// 			clientState.prepareNextRequestWithLeftover(leftover);
// 			std::cout << GREEN "DEBUG" << RESET 
// 					  << "[processData]: Case B, prepared next request with leftover ("
// 					  << leftover.size() << " bytes)\n";
// 		}
		
// 		// Adjust index because we just removed a request
// 		// --i; //for production
// 	}

// 	return anyRequestDone;
// }

bool ConnectionManager::processData(int clientId, const std::string& data)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return false;

	ClientState& clientState = it->second;
	ParsedRequest& req = clientState.getLatestRequest();

	if (clientState.getParsedRequestCount() > 0 && req.isRequestDone())
	{
		clientState.prepareForNextRequest();
	}
	
	// Append all incoming bytes to _tempBuffer
	req.appendTempBuffer(data);

	// Determine leftover bytes after headers
	std::string leftover;
	if (req.isHeadersDone() == false)
	{
		leftover = remainingAfterParsingHeaders(clientState);
		if (leftover.empty())
		{
			// Not enough bytes yet
			return false;
		}
	}
	appendBodyBytes(clientState, leftover);
	
	return hasCompletedRequests(clientState);
}

bool ConnectionManager::remainingAfterParsingHeaders(ClientState& clientState)
{
	ParsedRequest& request = clientState.getLatestRequest();
	const std::string& temp = request.getTempBuffer();

	// Check if we have a full header block (ends with "\r\n\r\n")
	size_t headerEnd = temp.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		// Not enough data yet
		return false;
	}

	// Extract the full header block
	std::string headerPart = temp.substr(0, headerEnd + 4); // include "\r\n\r\n"
	
	try
	{
		// Parse request line and headers
		request.parseRequestLineAndHeaders(headerPart);
	}
	catch (const std::exception& e)
	{
		request.setBodyType(BodyType::ERROR);
		request.setBodyDone();
		request.clearTempBuffer();
		throw std::runtime_error(std::string("Failed to parse headers: ") + e.what());
	}

	// Move any leftover bytes after headers into the appropriate buffer
	std::string leftover = temp.substr(headerEnd + 4);
	if (!leftover.empty())
	{
		// Depending on body type, append to correct buffer
		switch (request.getBodyType())
		{
			case BodyType::SIZED:
				request.appendToContentLengthBuffer(leftover);
				break;
			case BodyType::CHUNKED:
				request.appendToChunkedBuffer(leftover);
				break;
			case BodyType::NO_BODY:
			case BodyType::ERROR:
				break; // nothing to do
		}
	}

	// Clear temp buffer since headers are parsed
	request.clearTempBuffer();

	return true; // headers successfully parsed
}

void ConnectionManager::appendBodyBytes(ClientState& clientState, const std::string& data)
{
	ParsedRequest& request = clientState.getLatestRequest();

	switch (request.getBodyType())
	{
		case BodyType::SIZED:
			request.appendToContentLengthBuffer(data);
			if (request.contentLengthComplete())
				request.setBodyDone();
			break;

		case BodyType::CHUNKED:
		{
			request.appendToChunkedBuffer(data);
			std::string decoded = request.decodeChunkedBody();
			request.setBody(decoded);
			if (request.isTerminatingZeroChunkReceived())
				request.setBodyDone();
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
	bool anyRequestDone = false;

	for (size_t i = 0; i < clientState.getParsedRequestCount(); ++i)
	{
		ParsedRequest& req = clientState.getParsedRequest(i);
		
		// Stop if this request is not yet done
		if (!req.isRequestDone())
			break;

		//Handle finished request (send response, log, etc.)
		// Here we assume handleFinishedRequest manages sending and cleanup
		handleFinishedRequest(clientState, i);

		anyRequestDone = true;

		if (!req.isRequestDone())
		{
			break; // cannot process further requests until this one is done
		}

        // Optional: if you remove finished requests from the list
        // clientState.popFirstFinishedRequest();
        // --i; // adjust index since we removed a request
	}

	return anyRequestDone;
}

void ConnectionManager::handleFinishedRequest(ClientState& clientState, size_t index)
{
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

    // Mark request done
    finishedReq.setRequestDone();

    // If leftover exists, move it to a new request
    if (!leftover.empty())
    {
        ParsedRequest& newReq = clientState.addParsedRequest();
        newReq.appendTempBuffer(leftover);
    }
}

std::string ConnectionManager::distinguishHeadersFromBody(ClientState& clientState, size_t reqNum)
{
	std::string leftover = processHeaders(clientState, reqNum);
	processLeftover(clientState, reqNum, leftover);
	return leftover;
}

std::string ConnectionManager::processHeaders(ClientState& clientState, size_t reqNum)
{
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

void ConnectionManager::processLeftover(ClientState& clientState, size_t reqNum, std::string leftover)
{
	ParsedRequest& request = clientState.getParsedRequest(reqNum);

	if (leftover.empty())
	{
		// Nothing to do if no leftover
		return;
	}
	
	switch (request.getBodyType())
	{
		case BodyType::CHUNKED:
			request.appendToChunkedBuffer(leftover);
			leftover.clear();
			if (request.isTerminatingZeroChunkReceived())
			{
				std::string decoded = request.decodeChunkedBody();
				request.setBody(decoded);
				request.setBodyDone();
				request.clearChunkedBuffer();
			}
			break;

		case BodyType::SIZED:
			request.appendToContentLengthBuffer(leftover);
			leftover.clear();
			if (request.contentLengthComplete())
			{
				request.setBody(request.getContentLengthBuffer());
				request.setBodyDone();
				request.clearContentLengthBuffer();
				request.clearRlAndHeadersBuffer();
			}
			break;

		case BodyType::NO_BODY:
			// leftover belongs to next request
			request.setRlAndHeadersBuffer(leftover);
			clientState.prepareNextRequestWithLeftover(leftover);
			break;

		case BodyType::ERROR:
			throw std::runtime_error("Request body in error state");
			break;
	}
}





// bool ConnectionManager::accumulateBody(ParsedRequest& request, const std::string& chunk)
// {
// 	request.appendToBodyBuffer(chunk);

// 	size_t contentLength = request.getContentLength();
// 	size_t currentSize   = request.getBodyBuffer().size();

// 	std::cout << GREEN "DEBUG" << RESET << "[accumulateBody]: Appended chunk of size "
// 			  << chunk.size() << ", total body size = " << currentSize
// 			  << " / " << contentLength << std::endl;
	
// 	// If we have received all bytes according to Content-Length
// 	if (currentSize >= contentLength)
// 	{
// 		std::cout << GREEN << "DEBUG" << RESET << "[accumulateBody]: Body complete (Content-Length reached)." << std::endl;
		
// 		// Copy accumulated buffer into the final body of ParsedRequest
		
// 		std::cout << GREEN << "DEBUG" << RESET << "[accumulateBody]: _bodyBuffer before setBody(): '"
// 		  << request.getBodyBuffer() << "'" << std::endl;
// 		request.setBody(request.getBodyBuffer().substr(0, contentLength));
		
// 		std::cout << GREEN << "DEBUG" << RESET << "[accumulateBody]: getBody() after setBody(): '"
// 		  << request.getBody() << "'" << std::endl;
// 		request.setBodyDone();
		
// 		return true; //body complete
// 	}
// 	return false;
// }

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
	std::cout << "Request #" << i
			  << ": Method = " << req.getMethod()
			  << ", URI = " << req.getUri() << "\n";
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
}