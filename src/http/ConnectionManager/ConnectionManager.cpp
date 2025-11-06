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
		RequestContext ctx = m_config.createRequestContext(reqData.getHeader("Host"), reqData.uri);
		printReqContext(ctx);
		
		Response resp(reqData, ctx);

		// Handle Bad Request (400) first
		if (rawReq.isBadRequest()) 
		{
			std::cout << "[genRespsForReadyReqs] Bad request detected, generating 400 response\n";
			HttpStatusCode status_code = HttpStatusCode::BadRequest;
			resp.setStatus(status_code);
			
			// Check if a custom error page exists for 400
			auto itErr = ctx.error_pages.find(status_code);
            if (itErr != ctx.error_pages.end() && !itErr->second.empty())
			{
				// Custom 400 error page exists, do internal redirect
				std::string errorPageUri = itErr->second;
				
				// Create a new context for the error page
				RequestContext errorCtx = m_config.createRequestContext(reqData.getHeader("Host"), errorPageUri);

				// Ensure GET method is allowed for the internal error page request
				if (std::find(errorCtx.allowed_methods.begin(), errorCtx.allowed_methods.end(), HttpMethod::GET) == errorCtx.allowed_methods.end())
    				errorCtx.allowed_methods.push_back(HttpMethod::GET);
				
				// Create a new Response object for the error page
				Response errorResp(reqData, errorCtx);
				
				// Generate the response for the error page
				std::string finalResp = errorResp.genResp();
                if (finalResp.empty())
                {
					// genResp() returned empty: internal redirect signal
    				// Already handling internal redirect, so serve manually to avoid infinite loop
					std::cout << "[genRespsForReadyReqs] finalResp is empty\n";
                    errorResp.setErrorPageBody(status_code); // fallback body
                }

                clientState.enqueueResponse(errorResp);
                continue; // skip normal processing
			}
			else
            {	
				// No custom page: generate default 400 page
                resp.setErrorPageBody(status_code);
                clientState.enqueueResponse(resp);
                continue;
            }
		}

		// Normal request processing
		std::string result = resp.genResp();
		
		// Internal redirect for other errors (e.g., 403, 404, 405, 413, 500)
		HttpStatusCode status_code = resp.getStatusCode();
		
		std::cout << "[genRespsForReadyReqs] genResp() returned " 
                  << (result.empty() ? "\"\"" : "\"<non-empty>\"") 
                  << ", status_code = " << static_cast<int>(status_code) << "\n";
		
		// Check if internal redirect is needed
		if (result.empty() && status_code != HttpStatusCode::OK)
		{
            auto itErr = ctx.error_pages.find(status_code);
			
			std::string errorPageUri;

			if (itErr != ctx.error_pages.end() && !itErr->second.empty())
            {
				// Custom error page exists for this status
				errorPageUri = itErr->second;
            }
			else
            {
				// Fallback: try custom 404 page if original custom page doesn't exist
				auto it404 = ctx.error_pages.find(HttpStatusCode::NotFound);
				if (it404 != ctx.error_pages.end() && !it404->second.empty())
					errorPageUri = it404->second;
            }

			if (!errorPageUri.empty())
			{
				// INTERNAL REDIRECT: serve the custom error page
				std::cout << "[genRespsForReadyReqs] Internal redirect to custom error page: " 
                          << errorPageUri << "\n";

				RequestContext errorCtx = m_config.createRequestContext(reqData.getHeader("Host"), errorPageUri);
				if (std::find(errorCtx.allowed_methods.begin(), errorCtx.allowed_methods.end(), HttpMethod::GET) == errorCtx.allowed_methods.end())
    				errorCtx.allowed_methods.push_back(HttpMethod::GET);

				Response errorResp(reqData, errorCtx);
				
				std::string finalResp = errorResp.genResp();
				if (finalResp.empty())
				{
					// genResp() returned empty: serve manually to avoid infinite loop
    				std::cout << "[genRespsForReadyReqs] Serving custom error page manually\n";
    				errorResp.setErrorPageBody(status_code); // fallback body
				}

				clientState.enqueueResponse(errorResp);
			}
			else
            {
                // No custom page at all: fallback to default error page
                resp.setErrorPageBody(status_code);
                clientState.enqueueResponse(resp);
            }
		}
		else
        {
            // Normal 200 OK response
            clientState.enqueueResponse(resp);
        }
	}
}


// void ConnectionManager::handleErrorWithOptionalCustomPage(ClientState& clientState,
//     RequestData& reqData, RequestContext& ctx, Response& resp, HttpStatusCode code)
// {
//     std::cout << "[handleErrorWithOptionalCustomPage] Handling error code: " 
//               << static_cast<int>(code) << "\n";

//     // Step 1: set status on original Response
//     resp.setStatus(code);
//     std::cout << "[handleErrorWithOptionalCustomPage] resp status set to " 
//               << static_cast<int>(resp.getStatusCode()) << "\n";

//     // Step 2: look for a custom error page for this status code
//     auto itErr = ctx.error_pages.find(code);
//     std::string errorPageUri;

//     if (itErr != ctx.error_pages.end() && !itErr->second.empty())
//     {
//         errorPageUri = itErr->second;
//         std::cout << "[handleErrorWithOptionalCustomPage] Found custom page for code " 
//                   << static_cast<int>(code) << ": " << errorPageUri << "\n";
//     }
//     else if (code != HttpStatusCode::NotFound)
//     {
//         // fallback to 404
//         auto it404 = ctx.error_pages.find(HttpStatusCode::NotFound);
//         if (it404 != ctx.error_pages.end() && !it404->second.empty())
//         {
//             errorPageUri = it404->second;
//             std::cout << "[handleErrorWithOptionalCustomPage] No page for code " 
//                       << static_cast<int>(code) << ", fallback to 404: " 
//                       << errorPageUri << "\n";
//         }
//         else
//         {
//             std::cout << "[handleErrorWithOptionalCustomPage] No page for code " 
//                       << static_cast<int>(code) << " and no 404 fallback\n";
//         }
//     }

//     // Step 3: if we have a page, do internal redirect
//     if (!errorPageUri.empty())
//     {
//         std::cout << "[handleErrorWithOptionalCustomPage] Preparing internal redirect for: " 
//                   << errorPageUri << "\n";

//         RequestContext errorCtx = m_config.createRequestContext(reqData.getHeader("Host"), errorPageUri);

//         // ensure GET allowed
//         if (std::find(errorCtx.allowed_methods.begin(), errorCtx.allowed_methods.end(), HttpMethod::GET) == errorCtx.allowed_methods.end())
//         {
//             errorCtx.allowed_methods.push_back(HttpMethod::GET);
//             std::cout << "[handleErrorWithOptionalCustomPage] Added GET to allowed_methods for error page\n";
//         }

//         Response errorResp(reqData, errorCtx);

//         // generate response
//         std::string finalResp = errorResp.genResp();
//         std::cout << "[handleErrorWithOptionalCustomPage] genResp() returned " 
//                   << (finalResp.empty() ? "\"\"" : "\"<non-empty>\"") << "\n";

//         if (finalResp.empty())
//         {
//             // fallback if file doesn't exist
//             std::cout << "[handleErrorWithOptionalCustomPage] genResp() empty, serving manually\n";
//             errorResp.setErrorPageBody(code);
//         }

//         std::cout << "[handleErrorWithOptionalCustomPage] Enqueueing error response\n";
//         clientState.enqueueResponse(errorResp);
//     }
//     else
//     {
//         // no custom page: serve default
//         std::cout << "[handleErrorWithOptionalCustomPage] No custom page, generating default error page for code " 
//                   << static_cast<int>(code) << "\n";
//         resp.setErrorPageBody(code);
//         clientState.enqueueResponse(resp);
//     }
// }


// void ConnectionManager::genRespsForReadyReqs(int clientId)
// {
//     // Find the client state
//     auto it = m_clients.find(clientId);
//     if (it == m_clients.end())
//         return;

//     ClientState& clientState = it->second;

//     // Process all complete requests
//     while (clientState.hasCompleteRawRequest())
//     {
//         RawRequest rawReq = clientState.popFirstCompleteRawRequest();
//         RequestData reqData = rawReq.buildRequestData();
//         RequestContext ctx = m_config.createRequestContext(reqData.getHeader("Host"), reqData.uri);

//         printReqContext(ctx);

//         Response resp(reqData, ctx);

//         // Handle Bad Request (400) first
//         if (rawReq.isBadRequest())
//         {
//             std::cout << "[genRespsForReadyReqs] Bad request detected, generating 400 response\n";
//             handleErrorWithOptionalCustomPage(clientState, reqData, ctx, resp, HttpStatusCode::BadRequest);
//             continue;
//         }

//         // Generate normal response (genResp() will handle 413, 405, 404, etc.)
//         std::string result = resp.genResp();
//         HttpStatusCode status_code = resp.getStatusCode();

//         std::cout << "[genRespsForReadyReqs] genResp() returned "
//                   << (result.empty() ? "\"\"" : "\"<non-empty>\"")
//                   << ", status_code = " << static_cast<int>(status_code) << "\n";

//         // If genResp() returned empty and status is not OK, perform internal redirect
//         if (result.empty() && status_code != HttpStatusCode::OK)
//         {
//             std::cout << "[genRespsForReadyReqs] Response empty, handling error with internal redirect or fallback\n";
//             handleErrorWithOptionalCustomPage(clientState, reqData, ctx, resp, status_code);
//         }
//         else
//         {
//             // Normal 200 OK response
//             std::cout << "[genRespsForReadyReqs] Enqueueing normal response\n";
//             clientState.enqueueResponse(resp);
//         }
//     }
// }


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