#include "ConnectionManager.hpp"

// TODO: make m_config const once Config methods are const-correct
ConnectionManager::ConnectionManager(Config& config)
: m_config(config) {}

void ConnectionManager::addClient(int clientId)
{
	m_clients.emplace(clientId, ClientState());
}

// Remove a client
void ConnectionManager::removeClient(int clientId)
{
	m_clients.erase(clientId);
}


const RawRequest& ConnectionManager::getRawRequest(int clientId, size_t index) const
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("getRawRequest: clientId not found");

	if (index == SIZE_MAX)
		return it->second.getLatestRawReq(); // const getter
	
	return it->second.getRawRequest(index); // const getter
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
		return 0;

	ClientState& clientState = it->second;
		
	RawRequest& rawReq = clientState.getLatestRawReq(); // single parser now
	
	// Append all incoming bytes to _tempBuffer
	rawReq.appendTempBuffer(data);
	
	//std::cout << "[processReqs] tempBuffer is |" << rawReq.getTempBuffer() << "|\n";
	
	size_t parsedCount = 0;
	while(true)
	{
		RawRequest& rawReq = clientState.getLatestRawReq();
		
		//if we don’t yet have headers for this request, try to parse them.
		if (rawReq.isHeadersDone() == false)
		{
			rawReq.separateHeadersFromBody();
			if (rawReq.isBadRequest())
			{
				std::cout << "[genRespsForReadyReqs] Bad request detected\n";
			}
			if (rawReq.isHeadersDone() == false)
			{
				std::cout << "[processReqs]: headers are not finished yet\n";
				break; // Need more data for headers, exit loop until next call;
				
			}
		}
		//If headers are done, append body bytes if needed.
		if (!rawReq.isBadRequest() && rawReq.isHeadersDone() && rawReq.isBodyDone() == false)
		{
			rawReq.appendBodyBytes(rawReq.getTempBuffer());
			if (rawReq.isBodyDone() == false)
			{
				//std::cout << "[processReqs]: body not finished yet\n";
				break; // Need more data for body, exit loop until next call
			}
		}
		
		//std::cout << "[DEBUG] headersDone=" << rawReq.isHeadersDone()
          //<< " bodyDone=" << rawReq.isBodyDone()
          //<< " isBad=" << rawReq.isBadRequest() << "\n";
		  
		//At this point headers and body are done = full request parsed
		if ((rawReq.isHeadersDone() && rawReq.isBodyDone()) || rawReq.isBadRequest())
		{
			std::cout << "[processReqs]: setting request done" << "\n";
			rawReq.setRequestDone();
			parsedCount++;
			
			// --- Build RequestData for this fully parsed request ---
			// RequestData requestData = rawReq.buildRequestData();
			// clientState.addRequestData(requestData);
			std::string forNextReq = rawReq.getTempBuffer();
			// clientState.popRawReq();
			
			// Prepare for next request if leftover exists
			if (forNextReq.empty() == false)
			{
				//overwrite the buffer that store leftovers for next request
				std::cout << RED << "[processReqs]: forNextReq: " << RESET << "|"
					<< forNextReq << "|\n";

					
				std::cout << "[processReqs]: adding request" << "\n";
				RawRequest& newRawReq = clientState.addRawRequest();
				newRawReq.setTempBuffer(forNextReq);
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
	
	while (clientState.hasCompleteRawRequest())
	{
		RawRequest rawReq = clientState.popFirstCompleteRawRequest();
		RequestData reqData = rawReq.buildRequestData();
		// Build context using the host and URI from the request
		RequestContext ctx = m_config.createRequestContext(reqData.getHeader("Host"), reqData.uri);
		printReqContext(ctx);
		
		Response resp(reqData, ctx);
		
		if (rawReq.isBadRequest()) 
		{
			std::cout << "[ConnectionManager::genRespsForReadyReqs] Bad request detected, generating 400 response\n";
			resp.setStatus(static_cast<int>(HttpStatusCode::BadRequest)); //check code
			resp.setErrorPageBody(HttpStatusCode::BadRequest, ctx.error_pages);
		}
		
		else
			resp.genResp();
		clientState.enqueueResponse(resp);
	}
}

RawRequest ConnectionManager::popRawReq(int clientId)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		throw std::runtime_error("Client not found");
	
		
	return it->second.popRawReq();
}



Response ConnectionManager::popNextResponse(int clientId)
{
    auto it = m_clients.find(clientId);
    if (it == m_clients.end())
        throw std::runtime_error("No such client");

    return it->second.popNextResponse(); // calls ClientState method
}

bool ConnectionManager::hasPendingResponses(int clientId) const
{
	std::unordered_map<int, ClientState>::const_iterator it = m_clients.find(clientId);
	if (it == m_clients.end())
		return false;
	return it->second.hasPendingResponses();
}