#include "ClientState.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

ClientState::ClientState()
  : m_rawRequests()
  , m_respDataQueue()
{
	// The first empty request so getLatestRawReq() is always valid
	DBG("[ClientState Constructor] Creating first empty RawRequest so "
		"getLatestRawReq() is always valid");
	m_rawRequests.emplace_back();
}

ClientState::~ClientState()
{
	for (auto& cgi : m_activeCGIs)
	{
		if (cgi.fd_stdin != -1)
			close(cgi.fd_stdin);
		if (cgi.fd_stdout != -1)
			close(cgi.fd_stdout);
		kill(cgi.pid, SIGTERM);
	}
}

// ---------------------------ACCESSORS-----------------------------

bool ClientState::hasPendingResponseData() const
{
	return !m_respDataQueue.empty();
}

bool ClientState::hasCompleteRawRequest() const
{
	for (const auto& rawRequest : m_rawRequests)
		if (rawRequest.isRequestDone())
			return true;
	return false;
}

// In processReqs we always append bytes to the currently active reques.
// Will always return a request
RawRequest& ClientState::getLatestRawReq()
{
	if (m_rawRequests.empty())
	{
		DBG("[getLatestRawReq] _rawRequests empty, creating a new RawRequest");
		m_rawRequests.emplace_back(); // default-construct a new request
	}
	return m_rawRequests.back();
}

ResponseData& ClientState::frontResponseData()
{
	if (m_respDataQueue.empty())
		throw std::runtime_error("No pending responses");
	return m_respDataQueue.front();
}

const std::queue<ResponseData>& ClientState::getResponseQueue() const
{
	return m_respDataQueue;
}

// ---------------------------METHODS-----------------------------

void ClientState::enqueueResponseData(const ResponseData& resp)
{
	DBG("ResponseData queued");
	m_respDataQueue.push(resp);
}

RawRequest& ClientState::addRawRequest()
{
	DBG("[addRawRequest]: made a new request");
	m_rawRequests.emplace_back();
	return m_rawRequests.back();
}

RawRequest ClientState::popFrontRawRequest()
{
	if (m_rawRequests.empty() || !m_rawRequests.front().isRequestDone())
		throw std::runtime_error("No complete RawRequest available");

	RawRequest completed = std::move(m_rawRequests.front());
	m_rawRequests.pop_front();
	return completed;
}

void ClientState::popFrontResponseData()
{
	if (m_respDataQueue.empty())
		throw std::runtime_error("No pending responses");
	m_respDataQueue.pop();
}

ResponseData& ClientState::backResponseData()
{
	if (m_respDataQueue.empty())
		throw std::runtime_error("No response data in queue to peek.");
	return m_respDataQueue.back();
}

//CGI

std::vector<CGIData>& ClientState::getActiveCGIs()
{
	return m_activeCGIs;
}

CGIData& ClientState::createActiveCgi(RequestData& req, Client& client,
									  const std::string& interpreter,
									  const std::string& scriptPath,
									  ResponseData* resp)
{
	m_activeCGIs.emplace_back();
	CGIData& cgi = m_activeCGIs.back();

	cgi = CGIManager::startCGI(req, client, interpreter, scriptPath);
	cgi.response = resp;

	return cgi;
}

CGIData* ClientState::findCgiByPid(pid_t pid)
{
	for (auto& cgi : m_activeCGIs)
	{
		if (cgi.pid == pid)
			return &cgi;
	}
	return nullptr;
}

void ClientState::removeCgi(pid_t pid)
{
	auto it
		= std::remove_if(m_activeCGIs.begin(), m_activeCGIs.end(),
						 [pid](const CGIData& cgi) { return cgi.pid == pid; });
	if (it != m_activeCGIs.end())
		m_activeCGIs.erase(it, m_activeCGIs.end());
}

void ClientState::clearActiveCGIs()
{
	m_activeCGIs.clear();
}
