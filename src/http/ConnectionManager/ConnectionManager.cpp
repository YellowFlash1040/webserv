#include "ConnectionManager.hpp"

// TODO: make m_config const once Config methods are const-correct
ConnectionManager::ConnectionManager(const Config& config)
: m_config(config) {}

void ConnectionManager::addClient(Client& client)
{
    m_clients.emplace(&client, ClientState());
}

void ConnectionManager::removeClient(Client& client)
{
    m_clients.erase(&client);
}


const RawRequest& ConnectionManager::getRawRequest(Client& client, size_t index) const
{
    auto it = m_clients.find(&client);
    if (it == m_clients.end())
        throw std::runtime_error("getRawRequest: client not found");

    if (index == SIZE_MAX)
        return it->second.getLatestRawReq();
    
    return it->second.getRawRequest(index);
}

ClientState& ConnectionManager::getClientState(Client& client)
{
    auto it = m_clients.find(&client);
    if (it == m_clients.end())
        throw std::runtime_error("getClientState: client not found");

    return it->second;
}


bool ConnectionManager::processData(Client& client, const std::string& tcpData)
{

	// 1. Parse incoming TCP data
	size_t reqsNum = processReqs(client, tcpData);

	// 2. Generate responses for all ready requests
	if (reqsNum > 0)
		genResps(client);
				
	return reqsNum > 0;
}

size_t ConnectionManager::processReqs(Client& client, const std::string& data)
{
    std::cout << YELLOW << "DEBUG: processReqs: " << RESET  << std::endl;

    auto it = m_clients.find(&client);
    if (it == m_clients.end())
        return 0;

    ClientState& clientState = it->second;

    RawRequest& rawReq = clientState.getLatestRawReq(); // single parser now

    // Append all incoming bytes to _tempBuffer
    rawReq.appendTempBuffer(data);

    size_t parsedCount = 0;
    while (true)
    {
        RawRequest& rawReq = clientState.getLatestRawReq();

        // If headers are not yet complete, try to parse them
        if (!rawReq.isHeadersDone())
        {
            rawReq.separateHeadersFromBody();
            if (rawReq.isBadRequest())
                std::cout << "[processReqs] Bad request detected\n";

            if (!rawReq.isHeadersDone())
            {
                std::cout << "[processReqs]: headers are not finished yet\n";
                break; // need more data
            }
        }

        // If headers done, append body bytes if needed
        if (!rawReq.isBadRequest() && rawReq.isHeadersDone() && !rawReq.isBodyDone())
        {
            rawReq.appendBodyBytes(rawReq.getTempBuffer());
            if (!rawReq.isBodyDone())
                break; // need more data
        }

        // Full request parsed or bad request
        if ((rawReq.isHeadersDone() && rawReq.isBodyDone()) || rawReq.isBadRequest())
        {
            std::cout << "[processReqs]: setting request done\n";
            rawReq.setRequestDone();
            parsedCount++;

            std::string forNextReq = rawReq.getTempBuffer();

            // Prepare next request if leftover exists
            if (!forNextReq.empty())
            {
                std::cout << RED << "[processReqs]: forNextReq: " << RESET
                          << "|" << forNextReq << "|\n";

                std::cout << "[processReqs]: adding request\n";
                RawRequest& newRawReq = clientState.addRawRequest();
                newRawReq.setTempBuffer(forNextReq);
                continue;
            }
            else
            {
                break; // no leftover
            }
        }
        else
        {
            break; // not done yet
        }
    }

    return parsedCount;
}

void ConnectionManager::genResps(Client& client)
{
    std::cout << "[ConnectionManager::genResps] START\n";

    // Find the client state in the map
    auto it = m_clients.find(&client);
    if (it == m_clients.end())
        return; // Client not found, exit

    ClientState& clientState = it->second;

    const NetworkEndpoint& endpoint = client.getListeningEndpoint(); // теперь берем из клиента

    // Process all complete requests for this client
    while (clientState.hasCompleteRawRequest())
    {
        RawRequest rawReq = clientState.popFirstCompleteRawRequest();

        rawReq.printRequest();

        std::cout << "[genRespsForReadyReqs] Creating RequestContext with endpoint: "
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
        reqHandler.processRequest(rawReq, client, ctx);

        RawResponse curRawResp = clientState.popNextRawResponse();

        std::cout << "[genRespsForReadyReqs] Status: " 
                  << static_cast<int>(curRawResp.getStatusCode())
                  << ", internal redirect? " << curRawResp.isInternalRedirect() 
                  << "; external redirect? " << ctx.redirection.isSet << "\n";

        // Handle internal redirect
        if (curRawResp.isInternalRedirect())
        {
            std::cout << RED << "[genRespsForReadyReqs] INTERNAL REDIRECTION detected" << RESET << "\n";
            std::string newUri = reqHandler.getErrorPageUri(ctx, curRawResp.getStatusCode());
            std::cout << "[genRespsForReadyReqs] newUri = " << newUri << "\n";

            RequestContext newCtx = m_config.createRequestContext(endpoint, rawReq.getHost(), newUri);
            std::cout << "[genRespsForReadyReqs] Generated new ctx\n";

            FileHandler fileHandler(newCtx.index_files);
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
                enqueueInternRedirResp(newUri, newCtx, reqHandler, client, clientState);
            }

            // Pop the new RawResponse and convert it
            RawResponse nextResp = clientState.popNextRawResponse();
            nextResp.setStatusCode(curRawResp.getStatusCode());

            ResponseData curResData = nextResp.toResponseData();
            clientState.enqueueResponseData(curResData);
        }

        else if (curRawResp.isExternalRedirect())
        {
            std::cout << "[genRespsForReadyReqs] External redirect detected\n";

            // Prevent redirect loops
            if (ctx.redirection.url == curRawResp.getRedirectTarget())
            {
                std::cout << "[genRespsForReadyReqs] Trying to externally loop. No! " 
                          << curRawResp.getRedirectTarget() << "\n";
                reqHandler.enqueueErrorResponse(ctx, HttpStatusCode::MovedPermanently);
            }
            else
            {
                std::cout << "[genRespsForReadyReqs] Externally redirecting to " 
                          << curRawResp.getRedirectTarget() << "\n";
                std::string newUri = ctx.redirection.url;
                enqueueExternRedirResp(newUri, ctx, reqHandler, client);
            }
        }
        else
        {
            ResponseData curResData = curRawResp.toResponseData();
            clientState.enqueueResponseData(curResData);
        }
    }
}

void ConnectionManager::enqueueInternRedirResp(const std::string& newUri,
                                               RequestContext& ctx,
                                               RequestHandler& reqHandler,
                                               Client& client, 
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

	reqHandler.processRequest(dummyReq, client, ctx); 
}

void ConnectionManager::enqueueExternRedirResp(std::string& newUri, RequestContext newCtx, RequestHandler& reqHandler, Client& client)
{
	std::cout << "[enqueueExternRedirResp] External redirect to " << newUri << "\n";
	

	RawRequest redirReq;
	
	redirReq.setUri(newUri);
	//connection? 
	
	std::cout << "[enqueueExternRedirResp] External redirection...\n";
	
	reqHandler.processRequest(redirReq, client, newCtx); //enqueues
	
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

