#include "ClientState.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

ClientState::ClientState()
  : m_requests()
  ,  m_responses()
{
	// The first empty request so getLatestRawReq() is always valid
	DBG("[ClientState Constructor] Creating first empty RawRequest so "
		"getLatestRawReq() is always valid");
	m_requests.emplace_back();
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

bool ClientState::hasCompleteRequest() const
{
	for (const auto& rawRequest : m_requests)
		if (rawRequest.isRequestDone())
			return true;
	return false;
}

bool ClientState::hasPendingResponse() const
{
	return ! m_responses.empty();
}

// In processReqs we always append bytes to the currently active reques.
// Will always return a request
RawRequest& ClientState::backRequest()
{
	if (m_requests.empty())
	{
		DBG("[getLatestRawReq] _rawRequests empty, creating a new RawRequest");
		m_requests.emplace_back(); // default-construct a new request
	}
	return m_requests.back();
}

ResponseData& ClientState::backResponse()
{
	if ( m_responses.empty())
		throw std::runtime_error("No response data in queue to peek.");
	return  m_responses.back();
}

const ResponseData& ClientState::frontResponse() const
{
	if ( m_responses.empty())
		throw std::runtime_error("No pending responses");
	return  m_responses.front();
}

const std::queue<ResponseData>& ClientState::responses() const
{
	return  m_responses;
}

std::vector<CGIData>& ClientState::activeCGIs()
{
	return m_activeCGIs;
}

// ---------------------------METHODS-----------------------------

RawRequest& ClientState::addRequest()
{
	DBG("[addRawRequest]: made a new request");
	m_requests.emplace_back();
	return m_requests.back();
}

void ClientState::enqueueResponse(const ResponseData& resp)
{
	DBG("ResponseData queued");
	m_responses.push(resp);
}

RawRequest ClientState::popFrontRequest()
{
	if (m_requests.empty() || !m_requests.front().isRequestDone())
		throw std::runtime_error("No complete RawRequest available");

	RawRequest completed = std::move(m_requests.front());
	m_requests.pop_front();
	return completed;
}

void ClientState::popFrontResponse()
{
	if ( m_responses.empty())
		throw std::runtime_error("No pending responses");
	 m_responses.pop();
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
