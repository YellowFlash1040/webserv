#include "ConnectionManager.hpp"

// TODO: make m_config const once Config methods are const-correct
ConnectionManager::ConnectionManager(Config& config)
: m_config(config) {}

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
	{
		
		return false; // no requests yet
	}
		
	const ParsedRequest& req = clientState.getRequest(0);
	
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
		return it->second.getLatestRequest(); // const getter
	
	return it->second.getRequest(index); // const getter
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
			req.separateHeadersFromBody();
			if (req.isHeadersDone() == false)
			{
				std::cout << "[processReqs]: headers are not finished yet\n";
				break; // Need more data for headers, exit loop until next call;
				
			}
		}
		
		//If headers are done, append body bytes if needed.
		if (req.isHeadersDone() && req.isBodyDone() == false)
		{
			req.appendBodyBytes(req.getTempBuffer());
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

    while (clientState.getParsedRequestCount() > 0) 
    {
        ParsedRequest& req = clientState.getRequest(0);

        if (!req.isRequestDone() || !req.needsResponse())
            break; // stop at first incomplete request
		
		RequestContext ctx = m_config.createRequestContext(req.getHost(), req.getUri());
        printReqContext(ctx);
		
		Response resp(req, ctx);
		clientState.enqueueResponse(resp);

        std::string output = resp.genResp();
        
        req.setResponseAdded();
	
		popFinishedReq(clientId);

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

	ParsedRequest &req = clientState.getRequest(0);

	if (!req.isRequestDone())
		return ""; // nothing ready

	// Simple hardcoded response for testing
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";

	return response;
}


ParsedRequest ConnectionManager::popFinishedReq(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("Client not found");

	return it->second.popFirstFinishedReq();
}

bool ConnectionManager::isPathInsideRoot(const std::string& root, const std::string& resolved)
{
    try
	{
        std::filesystem::path rootPath = std::filesystem::canonical(root);
        std::filesystem::path filePath = std::filesystem::weakly_canonical(std::filesystem::path(root) / resolved);

        return std::mismatch(rootPath.begin(), rootPath.end(), filePath.begin()).first == rootPath.end();
    }
    catch (const std::filesystem::filesystem_error&)
	{
        return false;
    }
}