#include "ClientState.hpp"

ClientState::ClientState()
	: _rawRequests()
	, _responseQueue()
{
	// The first empty request so getLatestRawReq() is always valid
	std::cout << RED << "[ClientState Constructor] Creating first empty RawRequest so getLatestRawReq() is always valid\n";
	_rawRequests.emplace_back();
}

size_t ClientState::getRawReqCount() const
{
	return _rawRequests.size();
}

size_t ClientState::getRequestCount() const
{
	return _requestsData.size();
}


bool ClientState::latestRawReqNeedsBody() const
{
	if (_rawRequests.empty())
		return false;
	const RawRequest& latest = _rawRequests.back();

	// Needs a body if headers are done but body isn’t
	return latest.isHeadersDone() && !latest.isBodyDone();
}

void ClientState::enqueueResponse(const Response& resp)
{
	_responseQueue.push(resp);
}

bool ClientState::responseQueueEmpty() const
{
	return _responseQueue.empty();
}

const Response& ClientState::getRespObj() const
{
	if (_responseQueue.empty())
		throw std::runtime_error("No responses in queue");
	return _responseQueue.front();
}

//will always return a request
RawRequest& ClientState::getLatestRawReq()
{
	if (_rawRequests.empty())
	{
		 std::cout << RED 
			<< "[getLatestRawReq] _rawRequests empty, creating a new RawRequest" 
			<< RESET << "\n";
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
	std::cout << RED << "[addRawRequest]: made a new request" << RESET << "\n";
	_rawRequests.emplace_back();
	return 
		_rawRequests.back();

}

//requests
RawRequest& ClientState::getRawRequest(size_t idx)
{
	if (idx >= _rawRequests.size()) 
	{
		std::cout << "Tried to access raw request " << idx << " but size=" << _rawRequests.size() << "\n";
		throw std::out_of_range("getRawRequest index out of bounds");
	}
	return _rawRequests[idx];
}

const RawRequest& ClientState::getRawRequest(size_t idx) const
{
	return _rawRequests.at(idx);
}

RequestData& ClientState::getRequest(size_t idx)
{
	if (idx >= _requestsData.size()) 
	{
		std::cout << "Tried to access request " << idx << " but size=" << _requestsData.size() << "\n";
		throw std::out_of_range("getRequest index out of bounds");
	}
	return _requestsData[idx];
}

size_t ClientState::getLatestRawReqIndex() const
{
	return _rawRequests.empty() ? 0 : _rawRequests.size() - 1;
}

bool ClientState::hasPendingResponses() const
{
	return !_responseQueue.empty();
}

Response ClientState::popNextResponse()
{
	if (_responseQueue.empty())
		throw std::runtime_error("No pending responses to pop");

	Response resp = std::move(_responseQueue.front());
    _responseQueue.pop();
	return resp;
}

RawRequest ClientState::popRawReq()
{
	std::cout << YELLOW << "DEBUG: popRawReq:" << RESET << std::endl;
	std::cout << "[popRawReq]: _rawRequests size = " << _rawRequests.size() << "\n";

	for (size_t idx = 0; idx < _rawRequests.size(); ++idx)
	{
		auto& req = _rawRequests[idx];

		if (req.isRequestDone())
		{
			std::cout << RED << "[popRawReq]: Popping finished raw request #" << idx << RESET << "\n";

			RawRequest finished = std::move(req);
			_rawRequests.erase(_rawRequests.begin() + idx);
			return finished;
		}
	}

	std::cout << "No finished request found in _rawRequests\n";
	throw std::runtime_error("No finished request found");
}

RequestData ClientState::popRequestData()
{
	std::cout << YELLOW << "DEBUG: popRequestData" << RESET << std::endl;
	std::cout << "[popRequestData]: _requestsData size = " << _requestsData.size() << "\n";

	for (size_t idx = 0; idx < _requestsData.size(); ++idx)
	{
		auto& req = _requestsData[idx];

		{
			std::cout << RED << "[popRequestData]: Popping finished ready request #" << idx << RESET
				// << ", Method = " << req. //print the enum
				<< ", URI = " << req.uri
				<< ", Query = " << req.query
				<< ", Body = |" << req.body << "|"
				<< "\n\n";

			RequestData finished = std::move(req);
			_requestsData.erase(_requestsData.begin() + idx);
			return finished;
		}
	}

	std::cout << "No finished request found in _rawRequests\n";
	throw std::runtime_error("No finished request found");
	
}



void ClientState::addRequestData(const RequestData& requestData)
{
	std::cout << RED << "added Request to _requestData" << RESET << "\n";
     _requestsData.emplace_back(std::move(requestData));
}

bool ClientState::hasCompleteRawRequest() const {
    for (std::vector<RawRequest>::const_iterator it = _rawRequests.begin(); it != _rawRequests.end(); ++it) {
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