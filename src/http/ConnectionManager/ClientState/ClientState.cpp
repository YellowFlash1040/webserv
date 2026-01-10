#include "ClientState.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

ClientState::ClientState()
  : _rawRequests()
  , _respDataQueue()
{
    // The first empty request so getLatestRawReq() is always valid
    DBG("[ClientState Constructor] Creating first empty RawRequest so "
        "getLatestRawReq() is always valid");
    _rawRequests.emplace_back();
}

// ---------------------------ACCESSORS-----------------------------

bool ClientState::hasPendingResponseData() const
{
    return !_respDataQueue.empty();
}

bool ClientState::hasCompleteRawRequest() const
{
    for (const auto& rawRequest : _rawRequests)
        if (rawRequest.isRequestDone())
            return true;
    return false;
}

// In processReqs we always append bytes to the currently active reques.
// Will always return a request
RawRequest& ClientState::getLatestRawReq()
{
    if (_rawRequests.empty())
    {
        DBG("[getLatestRawReq] _rawRequests empty, creating a new RawRequest");
        _rawRequests.emplace_back(); // default-construct a new request
    }
    return _rawRequests.back();
}

ResponseData& ClientState::frontResponseData()
{
    if (_respDataQueue.empty())
        throw std::runtime_error("No pending responses");
    return _respDataQueue.front();
}

const std::queue<ResponseData>& ClientState::getResponseQueue() const
{
    return _respDataQueue;
}

// ---------------------------METHODS-----------------------------

void ClientState::enqueueResponseData(const ResponseData& resp)
{
    DBG("ResponseData queued");
    _respDataQueue.push(resp);
}

RawRequest& ClientState::addRawRequest()
{
    DBG("[addRawRequest]: made a new request");
    _rawRequests.emplace_back();
    return _rawRequests.back();
}

RawRequest ClientState::popFirstCompleteRawRequest()
{
    if (_rawRequests.empty() || !_rawRequests.front().isRequestDone())
        throw std::runtime_error("No complete RawRequest available");

    RawRequest completed = std::move(_rawRequests.front());
    _rawRequests.pop_front();
    return completed;
}

void ClientState::popFrontResponseData()
{
    if (_respDataQueue.empty())
        throw std::runtime_error("No pending responses");
    _respDataQueue.pop();
}

ResponseData& ClientState::backResponseData()
{
    if (_respDataQueue.empty())
        throw std::runtime_error("No response data in queue to peek.");
    return _respDataQueue.back();
}

CGIManager::CGIData& ClientState::createActiveCgi(
    RequestData& req, Client& client, const std::string& interpreter,
    const std::string& scriptPath, ResponseData* resp)
{
    _activeCGIs.emplace_back();
    CGIManager::CGIData& cgi = _activeCGIs.back();

    cgi = CGIManager::startCGI(req, client, interpreter, scriptPath);
    cgi.response = resp;

    return cgi;
}

CGIManager::CGIData* ClientState::findCgiByPid(pid_t pid)
{
    for (auto& cgi : _activeCGIs)
    {
        if (cgi.pid == pid)
            return &cgi;
    }
    return nullptr;
}

void ClientState::removeCgi(pid_t pid)
{
    auto it = std::remove_if(
        _activeCGIs.begin(), _activeCGIs.end(),
        [pid](const CGIManager::CGIData& cgi) { return cgi.pid == pid; });
    if (it != _activeCGIs.end())
        _activeCGIs.erase(it, _activeCGIs.end());
}

void ClientState::clearActiveCGIs()
{
    _activeCGIs.clear();
}
