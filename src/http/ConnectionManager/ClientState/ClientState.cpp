#include "ClientState.hpp"

ClientState::ClientState()
    : _rawRequests()
    , _respDataQueue()
{
    // The first empty request so getLatestRawReq() is always valid
    DBG("[ClientState Constructor] Creating first empty RawRequest so getLatestRawReq() is always valid");
    _rawRequests.emplace_back();
}

// size_t ClientState::getRawReqCount() const
// {
// 	return _rawRequests.size();
// }

// bool ClientState::latestRawReqNeedsBody() const
// {
// 	if (_rawRequests.empty())
// 		return false;
// 	const RawRequest& latest = _rawRequests.back();

// 	// Needs a body if headers are done but body isn’t
// 	return latest.isHeadersDone() && !latest.isBodyDone();
// }

void ClientState::enqueueResponseData(const ResponseData& resp)
{
    DBG("ResponseData queued");
    _respDataQueue.push(resp);
}

const ResponseData& ClientState::getRespDataObj() const
{
	if (_respDataQueue.empty())
		throw std::runtime_error("No responses in queue");
	return _respDataQueue.front();
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

const RawRequest& ClientState::getLatestRawReq() const
{
	if (_rawRequests.empty())
		throw std::runtime_error("No requests available in ClientState");
	return _rawRequests.back();
}

RawRequest& ClientState::addRawRequest()
{
    DBG("[addRawRequest]: made a new request");
    _rawRequests.emplace_back();
    return _rawRequests.back();
}

RawRequest& ClientState::getRawRequest(size_t idx)
{
    if (idx >= _rawRequests.size()) 
    {
        DBG("Tried to access raw request " << idx << " but size=" << _rawRequests.size());
        throw std::out_of_range("getRawRequest index out of bounds");
    }
    return _rawRequests[idx];
}

const RawRequest& ClientState::getRawRequest(size_t idx) const
{
	return _rawRequests.at(idx);
}

size_t ClientState::getLatestRawReqIndex() const
{
	return _rawRequests.empty() ? 0 : _rawRequests.size() - 1;
}

bool ClientState::hasPendingResponseData() const
{
	return !_respDataQueue.empty();
}

RawRequest ClientState::popRawReq()
{
    DBG("popRawReq:");
    DBG("_rawRequests size = " << _rawRequests.size());

    for (size_t idx = 0; idx < _rawRequests.size(); ++idx)
    {
        auto& req = _rawRequests[idx];

        if (req.isRequestDone())
        {
            DBG("Popping finished raw request #" << idx);

            RawRequest finished = std::move(req);
            _rawRequests.erase(_rawRequests.begin() + idx);
            return finished;
        }
    }

    DBG("No finished request found in _rawRequests");
    throw std::runtime_error("No finished request found");
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

RawResponse& ClientState::peekLastRawResponse()
{
	if (_rawResponsesQueue.empty())
		throw std::runtime_error("No raw responses in queue to peek.");
	return _rawResponsesQueue.back();
}

RawResponse ClientState::popNextRawResponse()
{
	if (_rawResponsesQueue.empty())
		throw std::runtime_error("No RawResponse to pop");

	RawResponse resp = std::move(_rawResponsesQueue.front());
	_rawResponsesQueue.pop_front();
	return resp;
}