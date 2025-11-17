#include "ConnectionManager.hpp"

// TODO: make m_config const once Config methods are const-correct
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
	
	// std::cout << YELLOW << "DEBUG: processReqs: " << RESET  << std::endl;
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
		
		//At this point headers and body are done = full request parsed
		if ((rawReq.isHeadersDone() && rawReq.isBodyDone()) || rawReq.isBadRequest())
		{
			std::cout << "[processReqs]: setting request done" << "\n";
			rawReq.setRequestDone();
			parsedCount++;
			
			std::string forNextReq = rawReq.getTempBuffer();
			
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
	




void ConnectionManager::genResps(int clientId, const NetworkEndpoint& endpoint)
{
	std::cout << "[ConnectionManager::genResps] START\n";

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
		
		std::cout << "[genRespsForReadyReqs] Creating RequestContext with endpoint: "
		//   << static_cast<std::string>(iface) << ":" << 8081 //getters?
		  << ", host: " << rawReq.getHost()
		  << ", uri: " << rawReq.getUri() << "\n";
		
		RequestContext ctx;
		try
		{
			ctx = m_config.createRequestContext(endpoint, rawReq.getHost(), rawReq.getUri());
		}
		catch (const std::exception& e)
		{
			std::cout << "[EXCEPTION] createRequestContext failed: " << e.what() << "\n";
			continue;
		}
		std::cout << "[genRespsForReadyReqs] RequestContext created successfully\n";
		
		printReqContext(ctx);
		
		// Create a RequestHandler for this client
		RequestHandler reqHandler(clientState);

		// Process the request (handles errors, CGI, files, etc.)
		reqHandler.processRequest(rawReq, endpoint, ctx);
		
		RawResponse curRawResp = clientState.popNextRawResponse();
		
		std::cout << "[genRespsForReadyReqs] Status: " 
          << static_cast<int>(curRawResp.getStatusCode())
          << ", internal redirect? " << curRawResp.isInternalRedirect() 
		  << "; external redirect? " << ctx.redirection.isSet << "\n";
		
		// Handle internal redirect
		if (curRawResp.isInternalRedirect())
		{	std::cout << RED << "[genRespsForReadyReqs] INTERNAL REDIRECTION deteced" << RESET << "\n";
			std::string newUri = reqHandler.getErrorPageUri(ctx, curRawResp.getStatusCode());
			std::cout << "[genRespsForReadyReqs] newUri = " << newUri << "\n";
			
			RequestContext newCtx = m_config.createRequestContext(endpoint, rawReq.getHost(), newUri);
			std::cout << "[genRespsForReadyReqs] Generated new ctx\n";
			
			
			FileHandler fileHandler(newCtx.autoindex_enabled, newCtx.index_files);
			if (!fileHandler.existsAndIsFile(newCtx.resolved_path))
			{
				std::cout << "Resolved error_page file does not exist\n";
				RawResponse redirResp;
				redirResp.addDefaultErrorDetails(curRawResp.getStatusCode());
				redirResp.addHeader("Content-Type", "text/html");
				redirResp.setInternalRedirect(false);
				clientState.enqueueRawResponse(redirResp);
							
				std::cout << "Trying to serve CUSTOM error page for code "
					<< static_cast<int>(curRawResp.getStatusCode()) << "\n";
				std::cout << "Resolved path " << ctx.resolved_path
					<< ctx.error_pages[curRawResp.getStatusCode()] << "\n";
			}
			else
			{	
				std::cout << "Resolved error_page file found\n";
				std::cout << "[genRespsForReadyReqs] Original code: " << static_cast<int>(curRawResp.getStatusCode()) << "\n";
				std::cout << "[genRespsForReadyReqs] Original Uri: " << ctx.resolved_path << "\n";
				std::cout << "[genRespsForReadyReqs] Error page path for this code: " 
          			<< ctx.error_pages[curRawResp.getStatusCode()] << "\n";
				
				enqueueInternRedirResp(newUri, newCtx, reqHandler, endpoint, clientState);

			}
	
			// Pop the new RawResponse and convert it
			RawResponse nextResp = clientState.popNextRawResponse();
			
			// Giving the served error page and error code instead of 200
			std::cout << "[genRespsForReadyReqs] Giving the served error page and error code instead of 200, new code: " << static_cast<int>(curRawResp.getStatusCode()) << "\n";
			nextResp.setStatusCode(curRawResp.getStatusCode());
			
			ResponseData curResData = nextResp.toResponseData();
			clientState.enqueueResponseData(curResData);
		}

		else if (curRawResp.isExternalRedirect())
		{
			std::cout << "[genRespsForReadyReqs] External redirect detected\n";

			//External redirect is looping
			if (ctx.redirection.url == curRawResp.getRedirectTarget())
			{
				std::cout << "[genRespsForReadyReqs] Trying to externally loop. No! " << curRawResp.getRedirectTarget() << "\n";
				reqHandler.enqueueErrorResponse(ctx, HttpStatusCode::MovedPermanently);
				
			}
			else
			{
				std::cout << "[genRespsForReadyReqs] Externaly redirecting to " << curRawResp.getRedirectTarget() << "\n";
				std::string newUri = ctx.redirection.url;
				enqueueExternRedirResp(newUri, ctx, reqHandler, endpoint);
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
                                               RequestContext& ctx,
                                               RequestHandler& reqHandler,
                                               const NetworkEndpoint& endpoint, 
                                               ClientState& clientState)
{
	
	(void)clientState;
    std::string fullPath;

    std::cout << "[enqueueInternRedirResp] Internal redirect to " << ctx.resolved_path << "\n";

	ctx.allowed_methods.push_back(HttpMethod::GET);
    
	// Prepare dummy GET request to fetch the error page
    RawRequest dummyReq;
	dummyReq.setMethod("GET");
	dummyReq.setUri(newUri);


	std::cout << "[enqueueInternRedirResp] dummyReq URI to GET: " << dummyReq.getUri() << "\n";

	reqHandler.processRequest(dummyReq, endpoint, ctx); 
}

void ConnectionManager::enqueueExternRedirResp(std::string& newUri, RequestContext newCtx, RequestHandler& reqHandler, const NetworkEndpoint& endpoint)
{
	std::cout << "[enqueueExternRedirResp] External redirect to " << newUri << "\n";
	

	RawRequest redirReq;
	
	redirReq.setUri(newUri);
	//connection? 
	
	std::cout << "[enqueueExternRedirResp] External redirection...\n";
	
	reqHandler.processRequest(redirReq, endpoint, newCtx); //enqueues
	
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

	std::cout << "[toResponseData] Enqueued RawResponse as ResponseData, status="
		<< data.statusCode << ", body_length=" << data.body.size() << "\n";

	return data;
}

