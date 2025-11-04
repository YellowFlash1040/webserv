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

		// Handle Bad Request (400) first
		if (rawReq.isBadRequest()) 
		{
			std::cout << "[genRespsForReadyReqs] Bad request detected, generating 400 response\n";
			HttpStatusCode status_code = HttpStatusCode::BadRequest;
			resp.setStatus(status_code);
			
			auto itErr = ctx.error_pages.find(status_code);
            if (itErr != ctx.error_pages.end() && !itErr->second.empty())
			{
				// internal redirect to error page
				std::string errorPageUri = itErr->second;
				RequestContext errorCtx = m_config.createRequestContext(reqData.getHeader("Host"), errorPageUri);
			  	Response errorResp(reqData, errorCtx);
				
				std::string finalResp = errorResp.genResp(true);
                if (finalResp.empty())
                {
					std::cout << "[genRespsForReadyReqs] finalResp is empty\n";
                    errorResp.setErrorPageBody(status_code);
                    finalResp = errorResp.toString();
                }

                clientState.enqueueResponse(errorResp);
                continue; // skip normal processing
			}
			else
            {
                resp.setErrorPageBody(status_code);
                clientState.enqueueResponse(resp);
                continue;
            }
		}
		// Normal request processing
		std::string result = resp.genResp();
		
		 // Internal redirect for other errors (e.g., 405, 413, 500)
		HttpStatusCode status_code = resp.getStatusCode();
		
		//There is a custom error page defined for this HTTP status code, and the file path is not empty
		if (result.empty() && status_code != HttpStatusCode::OK)
		{
            auto itErr = ctx.error_pages.find(status_code);
			if (itErr != ctx.error_pages.end() && !itErr->second.empty())
            {
				std::string errorPageUri = itErr->second;
                std::cout << "[genRespsForReadyReqs] Internal redirect to custom error page: "
                    << itErr->second << "\n";
				RequestContext errorCtx = m_config.createRequestContext(reqData.getHeader("Host"), errorPageUri);
				Response errorResp(reqData, errorCtx);
				
				std::string finalResp = errorResp.genResp();
				if (finalResp.empty())
				{
					// Serve static file for custom error page
    				std::cout << "[genRespsForReadyReqs] Serving custom error page statically\n";
    				finalResp = errorResp.handleStatic(); // serve the file content
				}

				clientState.enqueueResponse(errorResp);
				//continue; // skip normal enqueue of resp
			}
			  else
            {
                // No custom page, use default error page
                resp.setErrorPageBody(status_code);
                clientState.enqueueResponse(resp);
            }
		}
		else
        {
            // 4. Normal 200 OK response
            clientState.enqueueResponse(resp);
        }
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