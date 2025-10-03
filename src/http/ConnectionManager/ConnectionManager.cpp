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
bool ConnectionManager::processData(int clientId, const std::string& tcpData)
{

	// 1. Parse incoming TCP data
	size_t reqsNum = processReqs(clientId, tcpData);

	// 2. Generate responses for all ready requests
	if (reqsNum > 0)
    	genRespsForReadyReqs(clientId);
		
	return reqsNum > 0;
}

size_t ConnectionManager::processReqs(int clientId, const std::string& data)
{
	
	std::cout << YELLOW << "DEBUG: processReqs: " << RESET  << std::endl;
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return false;

	ClientState& clientState = it->second;
	std::cout << "[processReqs]: start of the function, requests:\n";
	printAllRequests(clientState);
	
	ParsedRequest& req = clientState.getLatestRequest();
	
	// Append all incoming bytes to _tempBuffer
	req.appendTempBuffer(data);
	
	std::cout << "[processReqs] tempBuffer is |" << req.getTempBuffer() << "|\n";
	
	size_t parsedCount = 0;
	while(true)
	{
		ParsedRequest& req = clientState.getLatestRequest();
		
		//if we don’t yet have headers for this request, try to parse them.
		if (req.isHeadersDone() == false)
		{
			separateHeadersFromBody(req);
			if (req.isHeadersDone() == false)
			{
                break; // Need more data for headers, exit loop until next call;
				std::cout << "[processReqs]: headers are not finished yet\n";
			}
		}
		
		//If headers are done, append body bytes if needed.
		if (req.isHeadersDone() && req.isBodyDone() == false)
		{
			appendBodyBytes(clientState, req.getTempBuffer());
			if (req.isBodyDone() == false)
			{
				std::cout << "[processReqs]: body not finished yet\n";
				break; // Need more data for body, exit loop until next call
			}
		}
		
		//At this point headers and body are done = full request parsed
		if (req.isHeadersDone() && req.isBodyDone())
        {
	
			std::cout << "[processReqs]: setting request done" << "\n";
            req.setRequestDone();
            parsedCount++;
			
			// Prepare for next request if leftover exists
        	if (req.getTempBuffer().empty() == false)
            {
				//overwrite the buffer that store leftovers for next request
				std::string forNextReq = req.getTempBuffer();
				std::cout << RED << "[processReqs]: forNextReq: " << RESET << "|"
					<< forNextReq << "|\n";
				
				req.setTempBuffer("");
					
				std::cout << "[processReqs]: adding request" << "\n";
				ParsedRequest& newReq = clientState.addParsedRequest();
				newReq.setTempBuffer(forNextReq);
				printAllRequests(clientState);
				continue;
				
            }
            else
            {
                break; // no leftover, stop processing
            }
		}
		else
        {
            break; // not done yet
        }
	}
	return (parsedCount);
}
	

void ConnectionManager::genRespsForReadyReqs(int clientId)
{
	auto it = m_clients.find(clientId);
    if (it == m_clients.end())
		return;
	
    ClientState& clientState = it->second;
	
	std::string allResponses;
	

    while (clientState.getParsedRequestCount() > 0) 
    {
		// printf ("In genRespsForReadyReqs:\n");
		// printAllRequests(clientState);
        ParsedRequest &req = clientState.getParsedRequest(0);

        if (!req.isRequestDone() || !req.needsResponse())
            break; // stop at first incomplete request

        // Generate/send the response for this request
         // Concatenate the response
        allResponses += genResp(clientId);
		req.setResponseAdded();  // mark as done
		//std::cout << "allResponses is: |" << allResponses << "|\n";
		
		// std::cout << "forNextRequest: |" << clientState.getForNextRequest()
		// << "|\n";
		//for production, comment out while testing:
		// popFinishedReq(clientId);

    }
}

std::string ConnectionManager::genResp(int clientId)
{
    auto it = m_clients.find(clientId);
    if (it == m_clients.end())
        return "";

    ClientState &clientState = it->second;

    if (clientState.getParsedRequestCount() == 0)
        return "";

    ParsedRequest &req = clientState.getParsedRequest(0);

    if (!req.isRequestDone())
        return ""; // nothing ready

    // Simple hardcoded response for testing
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";

    return response;
}

void ConnectionManager::separateHeadersFromBody(ParsedRequest& request)
{
	std::cout << YELLOW << "DEBUG: separateHeadersFromBody: " << RESET  << std::endl;
    const std::string& temp = request.getTempBuffer();
    std::cout << "[separateHeadersFromBody] tempBuffer = |" << temp << "|\n";

    size_t headerEnd = temp.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
	{
        std::cout << "[separateHeadersFromBody] Headers incomplete (\\r\\n\\r\\n not found)\n";
        return; // headers incomplete
    }

    std::string headerPart = temp.substr(0, headerEnd + 4);
    std::cout << "[separateHeadersFromBody] Header part = |" << headerPart << "|\n";

    try
    {
        request.parseRequestLineAndHeaders(headerPart);
    }
    catch (const std::exception& e)
    {
        std::cout << "[separateHeadersFromBody] Exception parsing headers: " << e.what() << "\n";
        request.setBodyType(BodyType::ERROR);
        request.setBodyDone();
        request.clearTempBuffer();
        throw;
    }
    catch (...)
    {
        std::cout << "[separateHeadersFromBody] Unknown exception parsing headers\n";
        request.setBodyType(BodyType::ERROR);
        request.setBodyDone();
        request.clearTempBuffer();
        throw;
    }

    if (request.getBodyType() == BodyType::NO_BODY)
    {
        std::cout << "No body, marking body and request done\n";
        request.setBodyDone();
        request.setRequestDone();
    }

    // Keep leftover (after headers) in tempBuffer
    std::string leftover = temp.substr(headerEnd + 4); // leftover for body and maybe next requests
    request.setTempBuffer(leftover);
    std::cout << "Temp buffer after headers removed = |" << request.getTempBuffer() << "|\n";
}



void ConnectionManager::appendBodyBytes(ClientState& clientState, const std::string& data)
{
	std::cout << YELLOW << "[DEBUG: appendBodyBytes]" << RESET << std::endl;
	ParsedRequest& request = clientState.getLatestRequest();

	std::cout << GREEN << "[appendBodyBytes] before appending:" << RESET << "\n"
		<< "ContentLengthBuffer() = " << request.getContentLengthBuffer() << "\n"
		<< "ChunkedBuffer() = " << request.getChunkedBuffer() << "\n";
		
	switch (request.getBodyType())
	{
		case BodyType::SIZED:
		{
			size_t remaining = request.getRemainingContentLength(); // bytes still needed
			std::cout << MINT << "[appendBodyBytes]: " << RESET "remaining bytes of content to append: "
				<< remaining << "\n";
			size_t toAppend = std::min(remaining, data.size());
			std::cout << MINT << "[appendBodyBytes]: " << RESET "bytes to will be appended in reality: "
				<< toAppend << "\n";
			request.appendToContentLengthBuffer(data.substr(0, toAppend));
			request.consumeTempBuffer(toAppend); // remove exactly what we consumed
			if (request.contentLengthComplete())
			{
				request.appendTobody(request.getContentLengthBuffer());
				request.setBodyDone();
			}
			std::cout << GREEN << "[appendBodyBytes] after appending:" << RESET << "\n"
			<< "ContentLengthBuffer() = " << request.getContentLengthBuffer() << "\n";
			std::cout << "[appendBodyBytes]: after finishing body length, requests:\n";
			printAllRequests(clientState);
			break;
		}

		case BodyType::CHUNKED:
		{
			request.appendToChunkedBuffer(request.getTempBuffer());
    		std::cout << GREEN << "[appendBodyBytes]: " << RESET
				<< "appeneded chunkBuffer with data from tempBuffer. It is now |"
				<< request.getChunkedBuffer() << "|\n";
			request.setTempBuffer(""); // consumed for decoding
			size_t bytesProcessed = 0;
			
			// decode as much as possible
			std::string decoded = request.decodeChunkedBody(bytesProcessed);
			request.appendToBody(decoded); // append only the decoded chunks
			
			// remove processed bytes from _chunkedBuffer
			request.setChunkedBuffer(request.getChunkedBuffer().substr(bytesProcessed));
			
			if (request.isTerminatingZero())
			{
				std::cout << GREEN << "[appendBodyBytes]: " << RESET
				<< "TerminatingZero found\n";
				request.setBodyDone();
				request.setTempBuffer(request.getChunkedBuffer());
				request.clearChunkedBuffer();
				std::cout << GREEN << "[appendBodyBytes]: " << RESET
					<< "set tempBuffer to the contents of hunkedBuffer, it is now = |"
					<< request.getTempBuffer() << "| and cleared chunkBuffer\n";
				
			}
			else
			{
				// partial chunk left? move leftovers to tempBuffer for next process
				request.setTempBuffer(request.getChunkedBuffer() + request.getTempBuffer());
				
				std::cout << GREEN << "[appendBodyBytes]: " << RESET
				<< "TerminatingZero not found yet\n";
			}
			break;
		}

		case BodyType::NO_BODY:
			// Nothing to append
			break;

		case BodyType::ERROR:
			throw std::runtime_error("Cannot append body data: request in ERROR state");
	}
	
}


ParsedRequest ConnectionManager::popFinishedReq(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("Client not found");

	return it->second.popFirstFinishedRequest();
}

std::string ConnectionManager::bodyTypeToString(BodyType t)
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

	ParsedRequest& req = clientState.getParsedRequest(i);
	std::cout << TEAL << "Request #" << i << RESET << "\n"
			  << "Method = " << req.getMethod()
			  << ", URI = " << req.getUri()
			  << ", BodyType = " << bodyTypeToString(req.getBodyType())
			  << ", HeadersDone = " << (req.isHeadersDone() ? "true" : "false")
			  << ", BodyDone = " << (req.isBodyDone() ? "true" : "false")
			  << ", RequestDone = " << (req.isRequestDone() ? "true" : "false")
			  << ", NeedsResponse = " << (req.needsResponse() ? "true" : "false") 
			  << "\n";
			  if (!req.getBody().empty())
				std::cout << "  Body: |" << req.getBody() << "|\n";
			if (!req.getTempBuffer().empty())
			{
				std::cout << "  tempBuffer: |" << req.getTempBuffer() << "|\n";
			}
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
	std::cout << std::endl;
}

// Print all requests in the ClientState
void ConnectionManager::printAllRequests(ClientState& clientState)
{
	size_t count = clientState.getParsedRequestCount();
	if (count == 0)
	{
		std::cout << "No requests in client state\n";
		return;
	}

	for (size_t i = 0; i < count; ++i)
	{
		printRequest(clientState, i);
	}
	std::cout << "\n";
}

//without last paramenter prints latest
void ConnectionManager::printSingleRequest(const ParsedRequest& req, size_t i)
{
    std::cout << TEAL << "Request #" << i << RESET << "\n"
              << "Method = " << req.getMethod()
              << ", URI = " << req.getUri()
              << ", BodyType = " << bodyTypeToString(req.getBodyType())
              << ", HeadersDone = " << (req.isHeadersDone() ? "true" : "false")
              << ", BodyDone = " << (req.isBodyDone() ? "true" : "false")
              << ", RequestDone = " << (req.isRequestDone() ? "true" : "false")
              << ", NeedsResponse = " << (req.needsResponse() ? "true" : "false")
              << "\n";

    if (!req.getBody().empty())
	{
        std::cout << "  Body: '" << req.getBody() << "'\n";
	}
		
    std::string host = req.getHeader("Host");
    std::string te   = req.getHeader("Transfer-Encoding");
    std::string cl   = req.getHeader("Content-Length");

    if (!host.empty()) std::cout << "  Host: " << host << "\n";
    if (!te.empty())   std::cout << "  Transfer-Encoding: " << te << "\n";
    if (!cl.empty())   std::cout << "  Content-Length: " << cl << "\n";
}













//for gtests
ParsedRequest ConnectionManager::popFinishedRequest(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("Client not found");

	return it->second.popFirstFinishedRequest();
}
void ConnectionManager::printBodyBuffers(ParsedRequest& req)
{
	std::cout << "=== Body Buffers for Request ===\n";
    std::cout << "TempBuffer:            |" << req.getTempBuffer() << "|\n";
    std::cout << "ContentLengthBuffer:   |" << req.getContentLengthBuffer() << "|\n";
    std::cout << "ChunkedBuffer:         |" << req.getChunkedBuffer() << "|\n";
    std::cout << "Final Body (_body):    |" << req.getBody() << "|\n";
    std::cout << "HeadersDone: " << req.isHeadersDone()
              << " BodyDone: " << req.isBodyDone()
              << " RequestDone: " << req.isRequestDone() << "\n";
}