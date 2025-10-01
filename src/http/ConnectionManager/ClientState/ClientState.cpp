#include "ClientState.hpp"
#include <stdexcept>

// ===== Constructor =====
ClientState::ClientState()
	: _parsedRequests()
	, _responseQueue()
	, _readyToSend(false)
{
	// Create the first empty request so getLatestRequest() is always valid
	_parsedRequests.emplace_back();
}

// ===== Request management =====

void ClientState::addParsedRequest(const ParsedRequest& req)
{
	_parsedRequests.push_back(req);
}

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

void ClientState::prepareForNextRequest()
{
	// Add new empty request for next pipelined message
	_parsedRequests.emplace_back();
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
	std::string leftoverBody = latest.getBodyBuffer();

	_parsedRequests.emplace_back(); // new empty request
	ParsedRequest& newReq = _parsedRequests.back();

	// Preserve leftover
	newReq.setRlAndHeadersBuffer(leftoverHeaders);
	newReq.appendToBodyBuffer(leftoverBody);
}

ParsedRequest& ClientState::getReqObj(size_t index)
{
	return _parsedRequests.at(index);
}

ParsedRequest& ClientState::getLatestReqObj()
{
	return _parsedRequests.back();
}

ParsedRequest ClientState::popFirstFinishedRequest()
{
	for (auto it = _parsedRequests.begin(); it != _parsedRequests.end(); ++it)
	{
		if (it->isBodyDone())
		{
			ParsedRequest finished = std::move(*it);
			_parsedRequests.erase(it);
			return finished;
		}
	}

	throw std::runtime_error("No finished request found");
}

ParsedRequest& ClientState::getParsedRequest(size_t index)
{
	if (index >= _parsedRequests.size())
		throw std::out_of_range("ParsedRequest index out of range");
	return _parsedRequests[index];
}

ParsedRequest& ClientState::addParsedRequest()
{
    _parsedRequests.emplace_back();
    return _parsedRequests.back();
}