#include "ConnectionManager.hpp"

// TODO: make m_config const once Config methods are const-correct
ConnectionManager::ConnectionManager(const Config& config)
  : m_config(config)
{
}

void ConnectionManager::addClient(int clientId)
{
    m_clients.emplace(clientId, ClientState());
}

// Remove a client
void ConnectionManager::removeClient(int clientId)
{
    m_clients.erase(clientId);
}

const RawRequest& ConnectionManager::getRawRequest(int clientId,
                                                   size_t index) const
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

bool ConnectionManager::processData(const NetworkEndpoint& endpoint,
                                    int clientId, const std::string& tcpData)
{

    // 1. Parse incoming TCP data
    size_t reqsNum = processReqs(clientId, tcpData);

    // 2. Generate responses for all ready requests
    if (reqsNum > 0)
        genRespsForReadyReqs(clientId, endpoint);

    return reqsNum > 0;
}

size_t ConnectionManager::processReqs(int clientId, const std::string& data)
{

    std::cout << YELLOW << "DEBUG: processReqs: " << RESET << std::endl;
    auto it = m_clients.find(clientId);
    if (it == m_clients.end())
        return 0;

    ClientState& clientState = it->second;

    RawRequest& rawReq = clientState.getLatestRawReq(); // single parser now

    // Append all incoming bytes to _tempBuffer
    rawReq.appendTempBuffer(data);

    // std::cout << "[processReqs] tempBuffer is |" << rawReq.getTempBuffer() <<
    // "|\n";

    size_t parsedCount = 0;
    while (true)
    {
        RawRequest& rawReq = clientState.getLatestRawReq();

        // if we don’t yet have headers for this request, try to parse them.
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
        // If headers are done, append body bytes if needed.
        if (!rawReq.isBadRequest() && rawReq.isHeadersDone()
            && rawReq.isBodyDone() == false)
        {
            rawReq.appendBodyBytes(rawReq.getTempBuffer());
            if (rawReq.isBodyDone() == false)
            {
                // std::cout << "[processReqs]: body not finished yet\n";
                break; // Need more data for body, exit loop until next call
            }
        }

        // At this point headers and body are done = full request parsed
        if ((rawReq.isHeadersDone() && rawReq.isBodyDone())
            || rawReq.isBadRequest())
        {
            std::cout << "[processReqs]: setting request done" << "\n";
            rawReq.setRequestDone();
            parsedCount++;

            std::string forNextReq = rawReq.getTempBuffer();

            // Prepare for next request if leftover exists
            if (forNextReq.empty() == false)
            {
                // overwrite the buffer that store leftovers for next request
                std::cout << RED << "[processReqs]: forNextReq: " << RESET
                          << "|" << forNextReq << "|\n";

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

void ConnectionManager::genRespsForReadyReqs(int clientId,
                                             const NetworkEndpoint& endpoint)
{
    (void)endpoint;

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
        RequestData reqData = rawReq.buildRequestData();

        // Build context using the host and URI from the request
        RequestContext ctx = m_config.createRequestContext(
            endpoint, reqData.getHeader("Host"), reqData.uri);
        printReqContext(ctx);

        RawResponse rawResp(reqData, ctx);

        // Handle Bad Request (400) first
        if (rawReq.isBadRequest())
        {
            HttpStatusCode statusCode = HttpStatusCode::BadRequest;

            // Internal redirect to custom 400 page
            auto itErr = ctx.error_pages.find(statusCode);
            if (itErr != ctx.error_pages.end() && !itErr->second.empty())
            {
                std::string errorPageUri = itErr->second;
                RequestContext errorCtx = m_config.createRequestContext(
                    endpoint, reqData.getHeader("Host"), errorPageUri);

                if (std::find(errorCtx.allowed_methods.begin(),
                              errorCtx.allowed_methods.end(), HttpMethod::GET)
                    == errorCtx.allowed_methods.end())
                {
                    errorCtx.allowed_methods.push_back(HttpMethod::GET);
                }

                RawResponse errorResp(reqData, errorCtx);
                errorResp.genResp();

                if (errorResp.isInternalRedirect())
                    errorResp.generateDefaultErrorPage(statusCode); // fallback

                clientState.enqueueResponseData(errorResp.toResponseData());
                continue;
            }
            else
            {
                // No custom page: fallback to default 400
                rawResp.generateDefaultErrorPage(statusCode);
                clientState.enqueueResponseData(rawResp.toResponseData());
                continue;
            }
        }

        // Normal request processing
        rawResp.genResp();

        // Internal redirect for other errors (e.g., 403, 404, 405, 413, 500)
        HttpStatusCode status_code = rawResp.getStatusCode();

        std::cout << "[genRespsForReadyReqs] genResp() returned "
                  << (rawResp.isInternalRedirect() ? "\"\"" : "\"<non-empty>\"")
                  << ", status_code = " << static_cast<int>(status_code)
                  << "\n";

        // Check if internal redirect is needed
        if (rawResp.isInternalRedirect() && status_code != HttpStatusCode::Ok)
        {
            // Internal redirect for other error codes
            auto itErr = ctx.error_pages.find(status_code);
            std::string errorPageUri;

            // Custom error page exists for this status
            if (itErr != ctx.error_pages.end() && !itErr->second.empty())
                errorPageUri = itErr->second;

            // Custom error page does not exist for this status
            else
            {
                auto it404 = ctx.error_pages.find(HttpStatusCode::NotFound);
                if (it404 != ctx.error_pages.end() && !it404->second.empty())
                {
                    errorPageUri = it404->second;
                    std::cout
                        << "[DEBUG] Using fallback 404 page: " << errorPageUri
                        << "\n";
                }
                else
                    std::cout << "[DEBUG] No fallback 404 page found, will use "
                                 "default error page\n";
            }

            if (!errorPageUri.empty())
            {
                // internal redirect: serve the custom error page
                std::cout << "[genRespsForReadyReqs] Internal redirect to "
                             "custom error page: "
                          << errorPageUri << "\n";

                RequestContext errorCtx = m_config.createRequestContext(
                    endpoint, reqData.getHeader("Host"), errorPageUri);
                if (std::find(errorCtx.allowed_methods.begin(),
                              errorCtx.allowed_methods.end(), HttpMethod::GET)
                    == errorCtx.allowed_methods.end())
                    errorCtx.allowed_methods.push_back(HttpMethod::GET);

                RawResponse errorResp(reqData, errorCtx);

                errorResp.genResp();
                if (errorResp.isInternalRedirect())
                {
                    // genResp() returned empty: serve manually to avoid
                    // infinite loop
                    std::cout << "[genRespsForReadyReqs] Serving custom error "
                                 "page manually\n";
                    errorResp.generateDefaultErrorPage(
                        status_code); // fallback body
                }

                clientState.enqueueResponseData(errorResp.toResponseData());
            }
            else
            {
                // No custom page at all: fallback to default error page
                std::cout << "[DEBUG] Will perform internal redirect to: "
                          << errorPageUri << "\n";
                rawResp.generateDefaultErrorPage(status_code);
                clientState.enqueueResponseData(rawResp.toResponseData());
            }
        }
        else
        {
            // Normal 200 OK response
            clientState.enqueueResponseData(rawResp.toResponseData());
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
