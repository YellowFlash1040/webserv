#include "ClientState.hpp"

// Constructor
ClientState::ClientState()
	: _headersComplete(false), _contentLength(0), _chunked(false), _readyToSend(false)
{
	// buffers are default-initialized
}

// Destructor
ClientState::~ClientState() {}

// Copy constructor
ClientState::ClientState(const ClientState& other)
	: _headerBuffer(other._headerBuffer),
	  _bodyBuffer(other._bodyBuffer),
	  _headersComplete(other._headersComplete),
	  _contentLength(other._contentLength),
	  _chunked(other._chunked),
	  _request(other._request),
	  _respObj(other._respObj),
	  _readyToSend(other._readyToSend) {}

// Copy assignment
ClientState& ClientState::operator=(const ClientState& other) {
	if (this != &other)
	{
		_headerBuffer = other._headerBuffer;
		_bodyBuffer = other._bodyBuffer;
		_headersComplete = other._headersComplete;
		_contentLength = other._contentLength;
		_chunked = other._chunked;
		_request = other._request;
		_respObj = other._respObj;
		_readyToSend = other._readyToSend;
	}
	return *this;
}

// Move constructor
ClientState::ClientState(ClientState&& other) noexcept
	: _headerBuffer(std::move(other._headerBuffer)),
	  _bodyBuffer(std::move(other._bodyBuffer)),
	  _headersComplete(other._headersComplete),
	  _contentLength(other._contentLength),
	  _chunked(other._chunked),
	  _request(std::move(other._request)),
	  _respObj(std::move(other._respObj)),
	  _readyToSend(other._readyToSend) {}

// Move assignment
ClientState& ClientState::operator=(ClientState&& other) noexcept {
	if (this != &other)
	{
		_headerBuffer = std::move(other._headerBuffer);
		_bodyBuffer = std::move(other._bodyBuffer);
		_headersComplete = other._headersComplete;
		_contentLength = other._contentLength;
		_chunked = other._chunked;
		_request = std::move(other._request);
		_respObj = std::move(other._respObj);
		_readyToSend = other._readyToSend;
	}
	return *this;
}

void ClientState::prepareForNextRequest()
{
	_headerBuffer.clear();
	_bodyBuffer.clear();
	_headersComplete = false;
	_contentLength = 0;
	_chunked = false;
	_request.requestReset();
	_readyToSend = false;
	// _respObj stays as-is until overwritten by next response
}

// Getters
const std::string& ClientState::getHeaderBuffer() const 
{
	return _headerBuffer;
}

const std::string& ClientState::getBodyBuffer() const
{
	return _bodyBuffer;
}

std::string ClientState::getFullRequestBuffer() const
{
	return _headerBuffer + _bodyBuffer;
}

const std::string& ClientState::getRawHeaderBuffer() const
{
	return _headerBuffer;
}

const std::string& ClientState::getRawBodyBuffer() const
{
	return _bodyBuffer;
}

bool ClientState::isHeadersComplete() const
{
	return _headersComplete;
}

size_t ClientState::getContentLength() const
{
	return _contentLength;
}

bool ClientState::isChunked() const
{
	return _chunked;
}

const Request& ClientState::getRequest() const
{
	return _request;
}

const Response& ClientState::getRespObj() const
{
	return _respObj;
}
bool ClientState::isReadyToSend() const
{
	return _readyToSend;
}

// Appenders
void ClientState::appendToHeaderBuffer(const std::string& data)
{
	_headerBuffer += data;
}

void ClientState::appendToBodyBuffer(const std::string& data) 
{
	_bodyBuffer += data;
}

// Setters
void ClientState::setHeadersComplete(bool value)
{
	_headersComplete = value;
}

void ClientState::setContentLength(int value)
{
	_contentLength = value;
}
void ClientState::setChunked(bool value)
{
	_chunked = value;
}
void ClientState::setRequest(const Request& req)
{
	_request = req;
}
void ClientState::setResponse(const Response& resp)
{
	_respObj = resp;
}
void ClientState::setReadyToSend(bool value)
{
	_readyToSend = value;
}

