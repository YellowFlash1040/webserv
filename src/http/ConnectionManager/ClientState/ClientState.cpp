#include "ClientState.hpp"
#include <stdexcept>

// ===== Constructor =====
ClientState::ClientState()
	: _parsedRequests()
	, _responseQueue()
	, _readyToSend(false)
{
	// The first empty request so getLatestRequest() is always valid
	std::cout << RED << "[ClientState Constructor] Creating first empty ParsedRequest so getLatestRequest() is always valid\n";
	_parsedRequests.emplace_back();
}

// ===== Request management =====

// void ClientState::addParsedRequest(const ParsedRequest& req)
// {
// 	 std::cout << RED << "[addParsedRequest]: Adding new ParsedRequest\n" << RESET << std::endl;
// 	_parsedRequests.push_back(req);
// }

size_t ClientState::getParsedRequestCount() const
{
	return _parsedRequests.size();
}

const ParsedRequest& ClientState::getReqObj(size_t index) const
{
	if (index >= _parsedRequests.size())
		throw std::out_of_range("Requested ParsedRequest index out of range");
	return _parsedRequests[index];
}

const ParsedRequest& ClientState::getLatestReqObj() const
{
	if (_parsedRequests.empty())
		throw std::runtime_error("No requests available in ClientState");
	return _parsedRequests.back();
}

bool ClientState::latestRequestNeedsBody() const
{
	if (_parsedRequests.empty())
		return false;
	const ParsedRequest& latest = _parsedRequests.back();

	// Needs a body if headers are done but body isn’t
	return latest.isHeadersDone() && !latest.isBodyDone();
}

// ===== Response management =====

void ClientState::enqueueResponse(const ServerResponse& resp)
{
	_responseQueue.push(resp);
}

bool ClientState::responseQueueEmpty() const
{
	return _responseQueue.empty();
}

ServerResponse ClientState::popNextResponse()
{
	if (_responseQueue.empty())
		throw std::runtime_error("No responses in queue");
	ServerResponse resp = _responseQueue.front();
	_responseQueue.pop();
	return resp;
}

const ServerResponse& ClientState::getRespObj() const
{
	if (_responseQueue.empty())
		throw std::runtime_error("No responses in queue");
	return _responseQueue.front();
}

// ===== Ready-to-send flag =====

void ClientState::setReadyToSend(bool value)
{
	_readyToSend = value;
}

bool ClientState::isReadyToSend() const
{
	return _readyToSend;
}

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

void ClientState::prepareNextRequestWithLeftover(const std::string& leftover)
{
	if (leftover.empty())
		return; // nothing to do

	ParsedRequest& current = getLatestReqObj();
	current.appendToRlAndHeaderBuffer(leftover);
}

void ClientState::finalizeLatestRequestBody()
{
	if (_parsedRequests.empty())
		return;

	ParsedRequest& latest = _parsedRequests.back();
	latest.setBodyDone();
}

void ClientState::prepareForNextRequestPreserveBuffers()
{
	// Create a new ParsedRequest but preserve leftover buffers from the current one
	if (_parsedRequests.empty())
	{
		_parsedRequests.emplace_back();
		return;
	}

	ParsedRequest& latest = _parsedRequests.back();
	std::string leftoverHeaders = latest.getRlAndHeadersBuffer();
	std::string leftoverBody = latest.getBody();

	_parsedRequests.emplace_back(); // new empty request
	ParsedRequest& newReq = _parsedRequests.back();

	// Preserve leftover
	newReq.setRlAndHeadersBuffer(leftoverHeaders);
	newReq.appendTobody(leftoverBody);
}

ParsedRequest& ClientState::getReqObj(size_t index)
{
	return _parsedRequests.at(index);
}

ParsedRequest& ClientState::getLatestReqObj()
{
	return _parsedRequests.back();
}

ParsedRequest& ClientState::getParsedRequest(size_t index)
{
	if (index >= _parsedRequests.size())
		throw std::out_of_range("ParsedRequest index out of range");
	return _parsedRequests[index];
}

ParsedRequest& ClientState::addParsedRequest()
{
	std::cout << RED << "[addParsedRequest]: made a new request" << RESET << "\n";
    _parsedRequests.emplace_back();
    return 
		_parsedRequests.back();

}

const std::string& ClientState::getForNextRequest() const
{
    return _forNextRequest;
}

void ClientState::setForNextRequest(const std::string& buf)
{
    _forNextRequest = buf;
}

void ClientState::appendForNextRequest(const std::string& more)
{
    _forNextRequest += more;
}

void ClientState::clearForNextRequest()
{
    _forNextRequest.clear();
}


//requests
ParsedRequest& ClientState::getRequest(size_t idx)
{
    return _parsedRequests.at(idx);
}

size_t ClientState::getLatestRequestIndex() const
{
    return _parsedRequests.empty() ? 0 : _parsedRequests.size() - 1;
}















//For gtests 
ParsedRequest ClientState::popFirstFinishedRequest()
{
	std::cout << YELLOW << "DEBUG: popFirstFinishedRequest(for GTESTS):" << RESET  << std::endl;
	std::cout << "[popFirstFinishedRequest]: _parsedRequests size = " << _parsedRequests.size() << "\n";
	
	for (size_t idx = 0; idx < _parsedRequests.size(); ++idx)
	{
		auto& req = _parsedRequests[idx];
		
        std::cout << "[popFirstFinishedRequest]: Request #" << idx
                  << ", Method = " << req.getMethod()
                  << ", URI = " << req.getUri()
                  << ", HeadersDone = " << req.isHeadersDone()
                  << ", BodyDone = " << req.isBodyDone()
                  << ", RequestDone = " << req.isRequestDone()
				  << ", NeedsResponse = " << (req.needsResponse() ? "true" : "false")
				  << "\n";
				  
		if (req.isRequestDone())
		{
			std::cout << RED << "-> [popFirstFinishedRequest]: Found finished request at index " << idx << ", POPPING it" 
				<< RESET << "\n";
			ParsedRequest finished = std::move(req);
			_parsedRequests.erase(_parsedRequests.begin() + idx);
			return finished;
		}
	}

	std::cout << "No finished request found in _parsedRequests\n";
    throw std::runtime_error("No finished request found");
}