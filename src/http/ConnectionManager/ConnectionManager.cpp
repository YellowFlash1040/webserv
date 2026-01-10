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
	DBG("m_clinets'client removed");
	m_clients.erase(clientId);
}

ClientState& ConnectionManager::getClientState(int clientId)
{
	return m_clients.at(clientId);
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
	DBG("DEBUG: processReqs: ");
	auto it = m_clients.find(client.getSocket());
	if (it == m_clients.end())
		return 0;

	ClientState& clientState = it->second;
	RawRequest& rawReq = clientState.getLatestRawReq();

	// Append all incoming bytes to temp buffer
	rawReq.appendTempBuffer(data);
	DBG("[processReqs] tempBuffer is |" << rawReq.getTempBuffer() << "|");

	size_t parsedCount = 0;

	while (true)
	{
		RawRequest& rawReq = clientState.getLatestRawReq();
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
			DBG("[processReqs]: leftovers exist, adding new RawRequest: |" << leftovers << "|");
			RawRequest& newReq = clientState.addRawRequest();
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
	auto it = m_clients.find(client.getSocket());
	if (it == m_clients.end())
		return; // client not found

	ClientState& clientState = it->second;

	// Process all complete raw requests for this client
	while (clientState.hasCompleteRawRequest())
	{
		// Pop the first complete raw request
		RawRequest rawReq = clientState.popFirstCompleteRawRequest();
		PrintUtils::printRawRequest(rawReq);

		// Call the separated processing function
		RawResponse rawResp = RequestHandler::handleSingleRequest(rawReq, client, m_config);

		// Convert RawResponse to ResponseData
		ResponseData data = rawResp.toResponseData();

		// Enqueue the response in the client state
		clientState.enqueueResponseData(data);
	}
}
