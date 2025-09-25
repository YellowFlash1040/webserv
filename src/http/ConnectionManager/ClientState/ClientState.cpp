#include "ClientState.hpp"

// Default constructor
ClientState::ClientState()
	: _headersDone(false),
	  _headersSeparatedFromBody(false),
	  _bodyDone(false),
	  _readyToSend(false),
	  _rlAndHeadersBuffer(),
	  _bodyBuffer(),
	  _contentLength(0),
	  _chunked(false),
	  _reqObj(),
	  _respObj()
{}

// Destructor
ClientState::~ClientState() {}

// Copy constructor
ClientState::ClientState(const ClientState& other)
	: _headersDone(other._headersDone),
	  _headersSeparatedFromBody(other._headersSeparatedFromBody),
	  _bodyDone(other._bodyDone),
	  _readyToSend(other._readyToSend),
	  _rlAndHeadersBuffer(other._rlAndHeadersBuffer),
	  _bodyBuffer(other._bodyBuffer),
	  _contentLength(other._contentLength),
	  _chunked(other._chunked),
	  _reqObj(other._reqObj),
	  _respObj(other._respObj)
{}

// Copy assignment
ClientState& ClientState::operator=(const ClientState& other)
{
	if (this != &other)
	{
		_headersDone = other._headersDone;
		_headersSeparatedFromBody = other._headersSeparatedFromBody;
		_bodyDone = other._bodyDone;
		_readyToSend = other._readyToSend;
		_rlAndHeadersBuffer = other._rlAndHeadersBuffer;
		_bodyBuffer = other._bodyBuffer;
		_contentLength = other._contentLength;
		_chunked = other._chunked;
		_reqObj = other._reqObj;
		_respObj = other._respObj;
	}
	return *this;
}

// Move constructor
ClientState::ClientState(ClientState&& other) noexcept
	: _headersDone(other._headersDone),
	  _headersSeparatedFromBody(other._headersSeparatedFromBody),
	  _bodyDone(other._bodyDone),
	  _readyToSend(other._readyToSend),
	  _rlAndHeadersBuffer(std::move(other._rlAndHeadersBuffer)),
	  _bodyBuffer(std::move(other._bodyBuffer)),
	  _contentLength(other._contentLength),
	  _chunked(other._chunked),
	  _reqObj(std::move(other._reqObj)),
	  _respObj(std::move(other._respObj))
{}

// Move assignment
ClientState& ClientState::operator=(ClientState&& other) noexcept
{
	if (this != &other)
	{
		_headersDone = other._headersDone;
		_headersSeparatedFromBody = other._headersSeparatedFromBody;
		_bodyDone = other._bodyDone;
		_readyToSend = other._readyToSend;
		_rlAndHeadersBuffer = std::move(other._rlAndHeadersBuffer);
		_bodyBuffer = std::move(other._bodyBuffer);
		_contentLength = other._contentLength;
		_chunked = other._chunked;
		_reqObj = std::move(other._reqObj);
		_respObj = std::move(other._respObj);
	}
	return *this;
}

// Reset all state for next request
void ClientState::prepareForNextRequest()
{
	_headersDone = false;
	_headersSeparatedFromBody = false;
	_bodyDone = false;
	_readyToSend = false;
	_rlAndHeadersBuffer.clear();
	_bodyBuffer.clear();
	_contentLength = 0;
	_chunked = false;
	_reqObj.requestReset();
	// _respObj stays until overwritten by next response
}

void ClientState::prepareForNextRequestBuffersOnly()
{
	_headersDone = false;
	_headersSeparatedFromBody = false;
	_bodyDone = false;
	_readyToSend = false;
	_rlAndHeadersBuffer.clear();
	_bodyBuffer.clear();
	_contentLength = 0;
	_chunked = false;

}

// Getters
const std::string& ClientState::getRlAndHeadersBuffer() const
{
	return _rlAndHeadersBuffer;
}

const std::string& ClientState::getBodyBuffer() const
{
	return _bodyBuffer;
}

std::string ClientState::getFullRequestBuffer() const
{
	return _rlAndHeadersBuffer + _bodyBuffer;
}

const ServerResponse& ClientState::getRespObj() const
{
	return _respObj;
}

const ParsedRequest& ClientState::getReqObj() const
{
	return _reqObj;
}

size_t ClientState::getContentLength() const
{
	return static_cast<size_t>(_contentLength);
}

// status queries
bool ClientState::isHeadersDone() const
{
	return _headersDone;
}

bool ClientState::hasHeadersBeenSeparatedFromBody() const
{
	return _headersSeparatedFromBody;
}


bool ClientState::isChunked() const
{
	return _chunked;
}

bool ClientState::isReadyToSend() const
{
	return _readyToSend;
}

bool ClientState::isBodyDone() const
{
	return _bodyDone;
}

// Appenders
void ClientState::appendToRlAndHeaderBuffer(const std::string& data)
{
	_rlAndHeadersBuffer += data;
}

void ClientState::appendToBodyBuffer(const std::string& data)
{
	_bodyBuffer += data;
}

// clear header buffer (useful when replacing header contents after splitting)
void ClientState::clearRlAndHeaderBuffer()
{
	_rlAndHeadersBuffer.clear();
}

// Setters (correct signatures)
void ClientState::setHeadersDone()
{
	_headersDone = true;
}

void ClientState::setBodyDone()
{
	_bodyDone = true;
}

void ClientState::setHeadersSeparatedFromBody()
{
	_headersSeparatedFromBody = true;
}

void ClientState::setContentLength(int value)
{
	_contentLength = value;
}

void ClientState::setChunked(bool value)
{
	_chunked = value;
}

void ClientState::setRequest(const ParsedRequest& req)
{
	_reqObj = req;
}

void ClientState::setResponse(const ServerResponse& resp)
{
	_respObj = resp;
}

void ClientState::setReadyToSend(bool value)
{
	_readyToSend = value;
}

void ClientState::appendToBuffers(const std::string& data)
{
	if (!_headersDone)
	{
		_rlAndHeadersBuffer.append(data);
		std::cout << "[DEBUG] appendToBuffers(): appended " << data.size()
			<< " bytes to rl+headers buffer (total=" << _rlAndHeadersBuffer.size() << ")\n";
	}
	else
	{
		_bodyBuffer.append(data);
		std::cout << "[DEBUG] appendToBuffers(): appended " << data.size()
			<< " bytes to body buffer (total=" << _bodyBuffer.size() << ")\n";
	}
}