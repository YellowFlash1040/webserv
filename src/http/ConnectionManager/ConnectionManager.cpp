#include "ConnectionManager.hpp"
#include "Server.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

ConnectionManager::ConnectionManager(const Config& config)
  : m_config(config)
{
}

// ---------------------------ACCESSORS-----------------------------

ClientState& ConnectionManager::clientState(int clientId)
{
	return m_clients.at(clientId);
}

// ---------------------------METHODS-----------------------------

/**
 * @brief Add a new client to the connection manager.
 * 
 * This function is called when a new client connects to the server.
 * 
 * @param clientId The socket descriptor identifying the client.
 * 
 * The function inserts a new entry into the `m_clients` map with `clientId` as the key.
 * A fresh `ClientState` object is constructed in place for this client, 
 * initializing its request queue, response queue, and active CGI list.
 * After this call, the map contains an entry equivalent to:
 * @code
 * m_clients[clientId] = ClientState();
 * @endcode
 */
void ConnectionManager::addClient(int clientId)
{
	m_clients.emplace(clientId, ClientState());
}

void ConnectionManager::removeClient(int clientId)
{
	DBG("m_clinets'client removed: " << clientId);
	m_clients.erase(clientId);
}

void ConnectionManager::processData(Client& client, const std::string& tcpData)
{
	// 1. Parse incoming TCP data
	size_t reqsNum = processReqs(client, tcpData);

	// 2. Generate responses for all ready requests
	if (reqsNum > 0)
		genResps(client);
}

/**
 * @brief Process incoming data for a client, parsing as many complete requests as possible.
 * 
 * The function handles two main cases:
 * 1. Partial (incomplete) requests that arrive over multiple TCP packets
 * 2. Pipelined requests, where multiple complete requests arrive in a single TCP chunk
 * 
 * Incomplete requests are kept in the client's temp buffer until more data arrives,
 * ensuring no bytes are lost. Leftover data after parsing a complete request
 * is moved to a new RawRequest, allowing multiple requests to be processed in one call.
 *
 * Example 1: Incomplete request
 * Client sends: "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n"
 *
 * - First read returns: "GET / HTTP."
 *   - appendTempBuffer stores it in the current RawRequest
 *   - parse() sees headers are incomplete → returns false
 *   - processReqs breaks the loop and returns 0 (no complete requests yet)
 *
 * - Control returns to ConnectionManager::processData()
 *   - genResps() is skipped because no complete requests were parsed
 *
 * - Control continues up to Server::processClient(), processEvent(), and monitorEvents()
 *   - The server continues monitoring other clients
 *   - The partially received request remains in the client state
 *
 * - Next TCP chunk arrives: "1\r\nHost: example.com\r\n\r\n"
 *   - appendTempBuffer adds these bytes to the existing RawRequest
 *   - parse() now sees headers are complete → returns true
 *   - parsedCount increments to 1
 *   - Responses can now be generated via genResps()
 *
 * Example 2: Pipelined requests
 * Client sends two requests in one chunk:
 * "GET /first HTTP/1.1\r\nHost: example.com\r\n\r\nGET /second HTTP/1.1\r\nHost: example.com\r\n\r\n"
 *
 * - appendTempBuffer stores the entire chunk in the current RawRequest
 * - parse() completes the first request → returns true
 * - Leftovers ("GET /second ...") are moved to a new RawRequest
 * - parse() now completes the second request → returns true
 * - parsedCount = 2, responses for both requests can be generated in one call to genResps()
 */
size_t ConnectionManager::processReqs(Client& client, const std::string& data)
{
	DBG("DEBUG: processReqs: ");
	auto it = m_clients.find(client.socket());
	if (it == m_clients.end())
		return 0;

	ClientState& clientState = it->second;
	RawRequest& rawReq = clientState.backRequest();

	// Append all incoming bytes to temp buffer
	rawReq.appendTempBuffer(data);
	DBG("[processReqs] tempBuffer is |" << rawReq.getTempBuffer() << "|");

	size_t parsedCount = 0;

	while (true)
	{
		RawRequest& rawReq = clientState.backRequest();
		bool done = rawReq.parse();

		if (!done)
		{
			// current request not complete, wait for more data
			break;
		}

		parsedCount++;

		// Check for leftovers (data after a complete request)
		std::string leftovers = rawReq.tempBuffer();
		if (!leftovers.empty())
		{
			DBG("[processReqs]: leftovers exist, adding new RawRequest: |"
				<< leftovers << "|");
			RawRequest& newReq = clientState.addRequest();
			newReq.setTempBuffer(leftovers);
			continue; // process the new request in the same loop
		}
		else
		{
			break; // no leftover, stop processing
		}
	}

	return parsedCount;
}

void ConnectionManager::genResps(Client& client)
{
	auto it = m_clients.find(client.socket());
	if (it == m_clients.end())
		return; // client not found

	ClientState& clientState = it->second;

	// Process all complete raw requests for this client
	while (clientState.hasCompleteRequest())
	{
		// Pop the first complete raw request
		RawRequest rawReq = clientState.popFrontRequest();
		PrintUtils::printRawRequest(rawReq);

		CgiRequestResult cgiResult;

		// Call the separated processing function
		RawResponse rawResp = RequestHandler::handleSingleRequest(
			rawReq, client, m_config, cgiResult);

		// Convert RawResponse to ResponseData
		ResponseData data = rawResp.toResponseData();

		if (rawReq.method() == HttpMethod::HEAD)
			data.body.clear();

		if (cgiResult.spawnCgi)
		{
			data.isReady = false;
			clientState.enqueueResponse(data);

			ResponseData& stored = clientState.backResponse();

			clientState.createActiveCgi(cgiResult.requestData, client,
										cgiResult.cgiInterpreter,
										cgiResult.cgiScriptPath, &stored);
			continue;
		}
		else
		{
			clientState.enqueueResponse(data);
		}
	}
}

CGIData* ConnectionManager::findCgiByStdoutFd(int fd)
{
	for (auto& pair : m_clients)
	{
		ClientState& state = pair.second;
		for (auto& cgi : state.activeCGIs())
		{
			if (cgi.fd_stdout == fd)
				return &cgi;
		}
	}
	return nullptr;
}

CGIData* ConnectionManager::findCgiByStdinFd(int fd)
{
	for (auto& pair : m_clients)
	{
		ClientState& state = pair.second;
		for (auto& cgi : state.activeCGIs())
		{
			if (cgi.fd_stdin == fd)
				return &cgi;
		}
	}
	return nullptr;
}

void ConnectionManager::onCgiExited(Server& server, pid_t pid, int status)
{
	if (WIFEXITED(status))
	{
		int exitCode = WEXITSTATUS(status);
		if (exitCode != 0)
			std::cerr << "[CGI] Process " << pid << " exited with code "
					  << exitCode << "\n";
	}
	else if (WIFSIGNALED(status))
	{
		int sig = WTERMSIG(status);
		std::cerr << "[CGI] Process " << pid << " killed by signal " << sig
				  << "\n";
	}

	for (auto& it : m_clients)
	{
		ClientState& state = it.second;

		CGIData* cgi = state.findCgiByPid(pid);
		if (!cgi)
			continue;

		RawResponse raw;
		if (!raw.parseFromCgiOutput(cgi->output))
			raw.addDefaultError(HttpStatusCode::InternalServerError);

		raw.setMimeType(raw.header("Content-Type"));

		*cgi->response = raw.toResponseData();
		// for ab test connection should be closed after CGI
		cgi->response->shouldClose = true;

		state.removeCgi(pid);

		server.cleanupCgiFds(*cgi);

		break;
	}
}
