#include "ClientState.hpp"

ClientState::ClientState()
	: _parsedRequests()
	, _responseQueue()
{
	// The first empty request so getLatestRequest() is always valid
	std::cout << RED << "[ClientState Constructor] Creating first empty ParsedRequest so getLatestRequest() is always valid\n";
	_parsedRequests.emplace_back();
}

size_t ClientState::getParsedRequestCount() const
{
	return _parsedRequests.size();
}


bool ClientState::latestRequestNeedsBody() const
{
	if (_parsedRequests.empty())
		return false;
	const ParsedRequest& latest = _parsedRequests.back();

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
ParsedRequest& ClientState::getLatestRequest()
{
	if (_parsedRequests.empty())
	{
		 std::cout << RED 
			<< "[getLatestRequest] _parsedRequests empty, creating a new ParsedRequest" 
			<< RESET << "\n";
		_parsedRequests.emplace_back(); // default-construct a new request
	}
	return _parsedRequests.back();
}

const ParsedRequest& ClientState::getLatestRequest() const
{
	if (_parsedRequests.empty())
		throw std::runtime_error("No requests available in ClientState");
	return _parsedRequests.back();
}

ParsedRequest& ClientState::addParsedRequest()
{
	std::cout << RED << "[addParsedRequest]: made a new request" << RESET << "\n";
	_parsedRequests.emplace_back();
	return 
		_parsedRequests.back();

}

//requests
ParsedRequest& ClientState::getRequest(size_t idx)
{
    if (idx >= _parsedRequests.size()) 
	{
        std::cerr << "Tried to access parsed request " << idx << " but size=" << _parsedRequests.size() << "\n";
        throw std::out_of_range("getRequest index out of bounds");
    }
    return _parsedRequests[idx];
}

const ParsedRequest& ClientState::getRequest(size_t idx) const
{
    return _parsedRequests.at(idx);
}

size_t ClientState::getLatestRequestIndex() const
{
	return _parsedRequests.empty() ? 0 : _parsedRequests.size() - 1;
}

//For gtests 
ParsedRequest ClientState::popFirstFinishedReq()
{
	std::cout << YELLOW << "DEBUG: popFirstFinishedRequest(for GTESTS):" << RESET << std::endl;
std::cout << "[popFirstFinishedRequest]: _parsedRequests size = " << _parsedRequests.size() << "\n";

	for (size_t idx = 0; idx < _parsedRequests.size(); ++idx)
	{
		auto& req = _parsedRequests[idx];

		if (req.isRequestDone())
		{
			std::cout << RED << "[popFirstFinishedRequest]: Popping finished request #" << idx << RESET
				<< ", Method = " << req.getMethod()
				<< ", rawURI = " << req.getRawUri()
				<< ", URI = " << req.getUri()
				<< ", Query = " << req.getQuery()
				<< ", HeadersDone = " << req.isHeadersDone()
				<< ", BodyDone = " << req.isBodyDone()
				<< ", RequestDone = " << req.isRequestDone()
				<< ", NeedsResponse = " << (req.needsResponse() ? "true" : "false")
				<< ", Body = |" << req.getBody() << "|"
				<< "\n";

			ParsedRequest finished = std::move(req);
			_parsedRequests.erase(_parsedRequests.begin() + idx);
			return finished;
		}
	}

	std::cout << "No finished request found in _parsedRequests\n";
	throw std::runtime_error("No finished request found");
}

