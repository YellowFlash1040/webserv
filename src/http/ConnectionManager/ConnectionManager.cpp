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
		std::string leftovers = rawReq.getTempBuffer();
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

		if (rawReq.getMethod() == HttpMethod::HEAD)
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

		raw.setMimeType(raw.getHeader("Content-Type"));

		*cgi->response = raw.toResponseData();
		// for ab test connection should be closed after CGI
		cgi->response->shouldClose = true;

		state.removeCgi(pid);

		server.cleanupCgiFds(*cgi);

		break;
	}
}
