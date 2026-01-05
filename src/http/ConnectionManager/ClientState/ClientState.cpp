#include "ClientState.hpp"

ClientState::ClientState()
    : _rawRequests()
    , _respDataQueue()
{
    // The first empty request so getLatestRawReq() is always valid
    DBG("[ClientState Constructor] Creating first empty RawRequest so getLatestRawReq() is always valid");
    _rawRequests.emplace_back();
}

void ClientState::enqueueResponseData(const ResponseData& resp)
{
    DBG("ResponseData queued");
    _respDataQueue.push(resp);
}

//will always return a request
RawRequest& ClientState::getLatestRawReq()
{
    if (_rawRequests.empty())
    {
        DBG("[getLatestRawReq] _rawRequests empty, creating a new RawRequest");
        _rawRequests.emplace_back(); // default-construct a new request
    }
    return _rawRequests.back();
}

RawRequest& ClientState::addRawRequest()
{
    DBG("[addRawRequest]: made a new request");
    _rawRequests.emplace_back();
    return _rawRequests.back();
}

bool ClientState::hasPendingResponseData() const
{
	return !_respDataQueue.empty();
}

bool ClientState::hasCompleteRawRequest() const
{
	for (std::deque<RawRequest>::const_iterator it = _rawRequests.begin(); it != _rawRequests.end(); ++it) {
		if (it->isRequestDone())
			return true;
	}
	return false;
}

RawRequest ClientState::popFirstCompleteRawRequest()
{
	for (size_t i = 0; i < _rawRequests.size(); ++i)
	{
		if (_rawRequests[i].isRequestDone())
		{
			RawRequest completed = std::move(_rawRequests[i]);
			_rawRequests.erase(_rawRequests.begin() + i);
			return completed;
		}
	}
	throw std::runtime_error("No complete RawRequest available");
}

ResponseData& ClientState::frontResponseData()
{
	if (_respDataQueue.empty())
		throw std::runtime_error("No pending responses");
	return _respDataQueue.front();
}

void ClientState::popFrontResponseData()
{
	if (_respDataQueue.empty())
		throw std::runtime_error("No pending responses");
	_respDataQueue.pop();
}

const std::queue<ResponseData>& ClientState::getResponseQueue() const
{
	return _respDataQueue;
}