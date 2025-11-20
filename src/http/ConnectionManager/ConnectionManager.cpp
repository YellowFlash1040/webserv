#include "ConnectionManager.hpp"

ConnectionManager::ConnectionManager(const Config& config)
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

ClientState& ConnectionManager::getClientState(int clientId)
{
	return m_clients.at(clientId);
}

bool ConnectionManager::processData(const NetworkEndpoint& endpoint, int clientId, const std::string& tcpData)
{

	// 1. Parse incoming TCP data
	size_t reqsNum = processReqs(clientId, tcpData);

	// 2. Generate responses for all ready requests
	if (reqsNum > 0)
		genResps(clientId, endpoint);
				
	return reqsNum > 0;
}

size_t ConnectionManager::processReqs(int clientId, const std::string& data)
{
	
	DBG(YELLOW << "DEBUG: processReqs: " << RESET);
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return 0;

	ClientState& clientState = it->second;
		
	RawRequest& rawReq = clientState.getLatestRawReq(); // single parser now
	
	// Append all incoming bytes to _tempBuffer
	rawReq.appendTempBuffer(data);
	
	DBG("[processReqs] tempBuffer is |" << rawReq.getTempBuffer() << "|");
	
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
				DBG("[genRespsForReadyReqs] Bad request detected");
			}
			if (rawReq.isHeadersDone() == false)
			{
				DBG("[processReqs]: headers are not finished yet");
				break; // Need more data for headers, exit loop until next call;
				
			}
		}
		//If headers are done, append body bytes if needed.
		if (!rawReq.isBadRequest() && rawReq.isHeadersDone() && rawReq.isBodyDone() == false)
		{
			rawReq.appendBodyBytes(rawReq.getTempBuffer());
			if (rawReq.isBodyDone() == false)
			{
				// DBG("[processReqs]: body not finished yet");
				break; // Need more data for body, exit loop until next call
			}
		}
		
		//At this point headers and body are done = full request parsed
		if ((rawReq.isHeadersDone() && rawReq.isBodyDone()) || rawReq.isBadRequest())
		{
			DBG("[processReqs]: setting request done");
			rawReq.setRequestDone();
			parsedCount++;
			
			std::string forNextReq = rawReq.getTempBuffer();
			
			// Prepare for next request if leftover exists
			if (forNextReq.empty() == false)
			{
				//overwrite the buffer that store leftovers for next request
				DBG("[processReqs]: forNextReq: |" << forNextReq << "|");
                DBG("[processReqs]: adding request");
				
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

void ConnectionManager::genResps(int clientId, const NetworkEndpoint& endpoint)
{
	 DBG("[ConnectionManager::genResps] START");

	// Find the client state in the map
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return; // Client not found, exit

	ClientState& clientState = it->second;
	
	// Process all complete requests for this client
	while (clientState.hasCompleteRawRequest())
	{
		// Pop the first complete raw request
		RawRequest rawReq = clientState.popFirstCompleteRawRequest();
		
		rawReq.printRequest();
		// Convert raw request into structured request data
		//RequestData reqData = rawReq.buildRequestData();
		
		// Resolve context using the host and URI from the request and NetworkEndpoint
		// NetworkInterface iface("0.0.0.0");
		// NetworkEndpoint endpoint(iface, 8081);
		
		DBG("[genResps] Creating RequestContext with host: "
            << rawReq.getHost() << ", uri: " << rawReq.getUri());
		
		RequestContext ctx;
		try
		{
			ctx = m_config.createRequestContext(endpoint, rawReq.getHost(), rawReq.getUri());
		}
		catch (const std::exception& e)
		{
			DBG("[genResps] createRequestContext failed: " << e.what());
			continue;
		}
		
		DBG("[genResps] RequestContext created successfully");
		printReqContext(ctx);
		
		// Create a RequestHandler for this client
		RequestHandler reqHandler(clientState);

		// Process the request (handles errors, CGI, files, etc.)
		reqHandler.processRequest(rawReq, endpoint, ctx);
		
		RawResponse curRawResp = clientState.popNextRawResponse();
		
		DBG("[genResps] Status: "
            << static_cast<int>(curRawResp.getStatusCode())
            << ", internal redirect? " << curRawResp.isInternalRedirect()
            << "; external redirect? " << ctx.redirection.isSet);
		
		// Handle internal redirect
		if (curRawResp.isInternalRedirect())
		{	
			DBG("[genResps] INTERNAL REDIRECTION detected");
			std::string newUri = reqHandler.getErrorPageUri(ctx, curRawResp.getStatusCode());
			DBG("[genResps] newUri = " << newUri);
			
			RequestContext newCtx = m_config.createRequestContext(endpoint, rawReq.getHost(), newUri);
			DBG("[genResps] Generated new ctx");
			
			FileHandler fileHandler(newCtx.index_files);
			if (!fileHandler.existsAndIsFile(newCtx.resolved_path))
			{
				DBG("[genResps] Resolved error_page file does not exist");
				
				RawResponse redirResp;
				redirResp.addDefaultErrorDetails(curRawResp.getStatusCode());
				redirResp.addHeader("Content-Type", "text/html");
				redirResp.setInternalRedirect(false);
				clientState.enqueueRawResponse(redirResp);
							
				DBG("[genResps] Trying to serve CUSTOM error page for code "
                    << static_cast<int>(curRawResp.getStatusCode())
                    << ", Resolved path: " << ctx.resolved_path
                    << ctx.error_pages[curRawResp.getStatusCode()]);
			}
			else
			{	
				DBG("[genResps] Resolved error_page file found");
                DBG("[genResps] Original code: " << static_cast<int>(curRawResp.getStatusCode()));
                DBG("[genResps] Original Uri: " << ctx.resolved_path);
                DBG("[genResps] Error page path for this code: "
                    << ctx.error_pages[curRawResp.getStatusCode()]);
				
				enqueueInternRedirResp(newUri, newCtx, reqHandler, endpoint, clientState);

			}
	
			// Pop the new RawResponse and convert it
			RawResponse nextResp = clientState.popNextRawResponse();
			
			// Giving the served error page and error code instead of 200
			DBG("[genResps] Giving the served error page and error code instead of 200, new code: "
                << static_cast<int>(curRawResp.getStatusCode()));
			
			nextResp.setStatusCode(curRawResp.getStatusCode());
			
			ResponseData curResData = nextResp.toResponseData();
			clientState.enqueueResponseData(curResData);
		}

		else if (curRawResp.isExternalRedirect())
		{
			DBG("[genResps] External redirect detected");

			//External redirect is looping
			DBG("[genResps]: curRawResp.getRedirectTarget(): " << ctx.redirection.url);
			DBG("[genResps]: rawReq.uri: " << rawReq.getUri());
			if (curRawResp.getRedirectTarget() == rawReq.getUri())
			{
 				DBG("[genResps] Trying to externally loop. No! " << curRawResp.getRedirectTarget());
				reqHandler.enqueueErrorResponse(ctx, HttpStatusCode::MovedPermanently);
				
			}
			else
			{
				DBG("[genResps] Externaly redirecting to " << curRawResp.getRedirectTarget());
				std::string newUri = ctx.redirection.url;
				RequestContext newCtx = m_config.createRequestContext(endpoint, rawReq.getHost(), newUri);
				//for the next request:
				// ctx.redirection.isSet = false;
				enqueueExternRedirResp(rawReq.getMethod(), newCtx, reqHandler, endpoint);
				RawResponse nextResp = clientState.popNextRawResponse();
				ResponseData curResData = nextResp.toResponseData();
				clientState.enqueueResponseData(curResData);
			}
		}
		
		else
		{
			ResponseData curResData = curRawResp.toResponseData();
			clientState.enqueueResponseData(curResData);
		}
	}
	// printAllResponses(clientState);
}

void ConnectionManager::enqueueInternRedirResp(const std::string& newUri,
                                               const RequestContext& ctx,
                                               RequestHandler& reqHandler,
                                               const NetworkEndpoint& endpoint, 
                                               ClientState& clientState)
{
	
	(void)clientState;
    std::string fullPath;
	
	RequestContext& mutableCtx = const_cast<RequestContext&>(ctx);
	mutableCtx.allowed_methods.push_back(HttpMethod::GET);

	//ctx.allowed_methods.push_back(HttpMethod::GET);
    
	// Prepare dummy GET request to fetch the error page
    RawRequest dummyReq;
	dummyReq.setMethod("GET");
	dummyReq.setUri(newUri);

	DBG("[enqueueInternRedirResp] dummyReq URI to GET: " << dummyReq.getUri());

	reqHandler.processRequest(dummyReq, endpoint, mutableCtx); 
}

void ConnectionManager::enqueueExternRedirResp(HttpMethod method, const RequestContext& newCtx, RequestHandler& reqHandler, const NetworkEndpoint& endpoint)
{
	DBG("[enqueueExternRedirResp] External redirect to " << newUri);

	RawRequest redirReq;
;
	redirReq.setMethod(method);

	// Optional: debug connection header if you add it
	// redirReq.addHeader("Connection", "keep-alive");
	// DBG("[enqueueExternRedirResp] Added Connection header: " << redirReq.getHeader("Connection"));

	// Debug: final state before processing
	DBG("[enqueueExternRedirResp] redirReq ready to process: reslved_path is " << newCtx.resolved_path);

	reqHandler.processRequest(redirReq, endpoint, newCtx); // enqueues
}





ResponseData toResponseData(const RawResponse& rawResp)
{
	ResponseData data;
	data.statusCode = static_cast<int>(rawResp.getStatusCode());
	data.statusText = rawResp.getStatusText();
	data.body = rawResp.getBody();
	data.headers = rawResp.getHeaders();

	if (!data.hasHeader("Content-Length"))
	{
		data.addHeader("Content-Length", std::to_string(data.body.size()));
	}

	std::string conn = rawResp.hasHeader("Connection") ? rawResp.getHeader("Connection") : "";
	if (conn == "close")
		data.shouldClose = true;
	else
		data.shouldClose = false;

	DBG("[toResponseData] Enqueued RawResponse as ResponseData, status="
        << data.statusCode << ", body_length=" << data.body.size());

	return data;
}

