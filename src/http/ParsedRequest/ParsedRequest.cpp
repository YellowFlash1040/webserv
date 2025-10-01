#include "ParsedRequest.hpp"
#include <sstream>
#include <algorithm>
#include <stdexcept>

// --- Helpers ---
void ParsedRequest::removeCarriageReturns(std::string& str)
{
	str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
}

void ParsedRequest::trimLeadingWhitespace(std::string& str)
{
	str.erase(
		str.begin(),
		std::find_if(str.begin(), str.end(),
					 [](unsigned char ch) { return !std::isspace(ch); })
	);
}

// --- Canonical form ---
ParsedRequest::ParsedRequest()
	: _tempBuffer(), _rlAndHeadersBuffer(), _bodyBuffer(), _chunkedBuffer(), _method(), _uri(), _httpVersion(),
	_headers(), _bodyType(BodyType::NO_BODY), _body(), _headersDone(false), _terminatingZeroReceived(false), _bodyDone(false),
	_requestDone(false), _contentLength(0), _chunked(false) {}

ParsedRequest::~ParsedRequest() = default;

ParsedRequest::ParsedRequest(const ParsedRequest& other) = default;
ParsedRequest& ParsedRequest::operator=(const ParsedRequest& other) = default;

ParsedRequest::ParsedRequest(ParsedRequest&& other) noexcept = default;
ParsedRequest& ParsedRequest::operator=(ParsedRequest&& other) noexcept = default;

// --- Getters ---
const std::string& ParsedRequest::getMethod() const { return _method; }
const std::string& ParsedRequest::getUri() const { return _uri; }
const std::string& ParsedRequest::getHttpVersion() const { return _httpVersion; }

std::string ParsedRequest::getHeader(const std::string& name) const
{
	auto it = _headers.find(name);
	return (it != _headers.end()) ? it->second : "";
}

const std::string& ParsedRequest::getRlAndHeadersBuffer() const { return _rlAndHeadersBuffer; }
std::string& ParsedRequest::getBodyBuffer()
{
	return _bodyBuffer;
}
size_t ParsedRequest::getContentLength() const { return _contentLength; }
bool ParsedRequest::isRequestDone() const { return _requestDone ;}
bool ParsedRequest::isHeadersDone() const { return _headersDone; }
bool ParsedRequest::isBodyDone() const { return _bodyDone; }
// bool ParsedRequest::isChunked() const { return _chunked; }

// --- Setters ---
void ParsedRequest::setMethod(const std::string& m) { _method = m; }
void ParsedRequest::setUri(const std::string& u) { _uri = u; }
void ParsedRequest::setHttpVersion(const std::string& v) { _httpVersion = v; }

void ParsedRequest::addHeader(const std::string& name, const std::string& value)
{
	auto it = _headers.find(name);
	if (it != _headers.end())
	{
		// Header already exists
		if (!iequals(it->second, value))
		{
			// Conflict: same header with different values
			_bodyType = BodyType::ERROR;
			throw std::runtime_error("Header conflict: " + name +
									 " has values [" + it->second + "] and [" + value + "]");
		}
		else
		{
			// Duplicate with same value: harmless
			return;
		}
	}
	_headers[name] = value;

}



void ParsedRequest::setHeadersDone() { _headersDone = true; }
void ParsedRequest::setBodyDone()
{
	_bodyDone = true;
}
void ParsedRequest::setRequestDone() { _requestDone = true; }
void ParsedRequest::setContentLength(int value) { _contentLength = value; }
void ParsedRequest::setChunked(bool value) { _chunked = value; }
void ParsedRequest::setRlAndHeadersBuffer(const std::string& newBuf) { _rlAndHeadersBuffer = newBuf; }

// --- Buffer manipulation ---
void ParsedRequest::appendToRlAndHeaderBuffer(const std::string& data) { _rlAndHeadersBuffer += data; }
void ParsedRequest::appendToBodyBuffer(const std::string& data) { _bodyBuffer += data; }
void ParsedRequest::appendToBuffers(const std::string& data)
{
	if (!_headersDone)
		_rlAndHeadersBuffer += data;
	else
		_bodyBuffer += data;
}
void ParsedRequest::clearRlAndHeaderBuffer() { _rlAndHeadersBuffer.clear(); }
void ParsedRequest::clearBodyBuffer() { _bodyBuffer.clear(); }


bool ParsedRequest::headersParsed() const
{
	return _rlAndHeadersBuffer.find("\r\n\r\n") != std::string::npos;
}

size_t ParsedRequest::extractContentLength() const
{
	// Look for Content-Length
	auto clIt = _headers.find("Content-Length");
	if (clIt == _headers.end())
		return 0; // no header: assume empty body

	try
	{
		long value = std::stol(clIt->second);
		if (value < 0)
			throw std::invalid_argument("Negative Content-Length not allowed");

		return static_cast<size_t>(value);
	}
	catch (const std::exception& e)
	{
		throw std::invalid_argument("Invalid Content-Length header: " + clIt->second);
	}
}


bool ParsedRequest::bodyComplete() const
{
	if (_chunked)
		return _bodyBuffer.find("\r\n0\r\n\r\n") != std::string::npos;
	else if (_contentLength > 0)
		return _bodyBuffer.size() >= static_cast<size_t>(_contentLength);
	else
		return true;
}

void ParsedRequest::finalizeBody()
{
	if (_chunked) _bodyBuffer = decodeChunkedBody();
	_bodyDone = true;
}

void ParsedRequest::parseRequestLineAndHeaders(const std::string& headerPart)
{
	std::istringstream stream(headerPart);
	std::string line;
	if (!std::getline(stream, line))
		throw std::runtime_error("Malformed request: missing request line");

	removeCarriageReturns(line);
	parseRequestLine(line);
	parseHeaders(stream);
}


void ParsedRequest::parseRequestLine(const std::string& firstLine)
{
	std::istringstream reqLine(firstLine);
	reqLine >> _method >> _uri >> _httpVersion;
	if (_method.empty() || _uri.empty() || _httpVersion.empty())
		throw std::runtime_error("Invalid request line");

	if (_method != "GET" && _method != "POST" && _method != "PUT" &&
		_method != "DELETE" && _method != "HEAD" && _method != "OPTIONS")
		throw std::invalid_argument("Unsupported HTTP method: " + _method);

	if (_httpVersion != "HTTP/1.0" && _httpVersion != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + _httpVersion);

	if (_uri[0] != '/')
		throw std::invalid_argument("Invalid request URI: " + _uri);
}
	
void ParsedRequest::parseHeaders(std::istringstream& stream)
{
	std::string line;
	while (std::getline(stream, line) && !line.empty() && line != "\r")
	{
		removeCarriageReturns(line);
		auto colonPos = line.find(':');
		if (colonPos == std::string::npos)
			throw std::runtime_error("Malformed header line: " + line);

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		trimLeadingWhitespace(value);
		try
		{
			addHeader(key, value);
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error("Header parse error: " + std::string(e.what()));
		}
	}

	// Decide body type
	if (iequals(getHeader("Transfer-Encoding"), "chunked"))
	{
		setBodyType(BodyType::CHUNKED);
	}
	else if (!getHeader("Content-Length").empty())
	{
		setBodyType(BodyType::SIZED);
	} 
	_contentLength = extractContentLength();
	if (_contentLength == -1)
		setBodyType(BodyType::ERROR);
	
	_headersDone = true;
	
}

bool ParsedRequest::iequals(const std::string& a, const std::string& b) const
{
	if (a.size() != b.size()) return false;
	for (size_t i = 0; i < a.size(); ++i)
		if (std::tolower(a[i]) != std::tolower(b[i]))
			return false;
	return true;
}

void ParsedRequest::clearRlAndHeadersBuffer()
{
	_rlAndHeadersBuffer.clear();
}

const std::string& ParsedRequest::getBody() const
{
	return _body;
}

void ParsedRequest::setBody(const std::string& body)
{
	_body = body;
}

// bool ParsedRequest::terminatingZeroChunkReceived() const
// {
// 	std::cout << GREEN << "DEBUG" << RESET
// 			  << "[terminatingZeroChunkReceived] Checking _chunkedBuffer of size " << _chunkedBuffer.size() << "\n";
	
			  
// 	std::cout << GREEN << "DEBUG" << RESET << "[terminatingZeroChunkReceived] _chunkedBuffer bytes (hex): ";
// 	for (unsigned char c : _chunkedBuffer)
// 		std::cout << std::hex << (int)c << " ";
// 	std::cout << std::dec << "\n"; // back to decimal
			  
// 	size_t pos = 0;

// 	while (true)
// 	{
// 		// Find the next CRLF (end of chunk size line)
// 		size_t crlfPos = _chunkedBuffer.find("\r\n", pos);
// 		if (crlfPos == std::string::npos)
// 		{
// 			std::cout << GREEN << "DEBUG" << RESET
// 					  << "[terminatingZeroChunkReceived] Incomplete chunk size line at pos " << pos << "\n";
// 			return false;
// 		}

// 		std::string sizeStr = _chunkedBuffer.substr(pos, crlfPos - pos);
// 		size_t chunkSize = 0;

// 		try
// 		{
// 			chunkSize = std::stoul(sizeStr, nullptr, 16);
// 		}
// 		catch (...)
// 		{
// 			std::cout << GREEN << "DEBUG" << RESET
// 					  << "[terminatingZeroChunkReceived] Malformed chunk size: '" << sizeStr << "'\n";
// 			return false;
// 		}

// 		pos = crlfPos + 2; // move past CRLF

// 		if (chunkSize == 0)
// 		{
// 			// Check if the trailing CRLF after the zero chunk is present
// 			if (_chunkedBuffer.size() >= pos + 2)
// 			{
// 				std::cout << GREEN << "DEBUG" << RESET
// 						  << "[terminatingZeroChunkReceived] Terminating zero chunk received with trailing CRLF\n";
// 				return true;
// 			}
// 			else
// 			{
// 				std::cout << GREEN << "DEBUG" << RESET
// 						  << "[terminatingZeroChunkReceived] Terminating zero chunk received, but trailing CRLF missing\n";
// 				return false;
// 			}
// 		}

// 		// Check if the full chunk + CRLF is present
// 		if (_chunkedBuffer.size() < pos + chunkSize + 2)
// 		{
// 			std::cout << GREEN << "DEBUG" << RESET
// 					  << "[terminatingZeroChunkReceived] Chunk data incomplete at pos " << pos
// 					  << " (need " << chunkSize + 2 << ", have " << _chunkedBuffer.size() - pos << ")\n";
// 			return false;
// 		}

// 		pos += chunkSize + 2; // move past chunk data and CRLF
// 	}
// }

std::string& ParsedRequest::getChunkedBuffer()
{
	return _chunkedBuffer;
}

void ParsedRequest::appendToChunkedBuffer(const std::string& data)
{
	std::cout << GREEN << "DEBUG" << RESET
			  << "[appendToChunkedBuffer] Before append, _chunkedBuffer size=" << _chunkedBuffer.size() << "\n";

	_chunkedBuffer += data;

	std::cout << GREEN << "DEBUG" << RESET
			  << "[appendToChunkedBuffer] After append, _chunkedBuffer size=" << _chunkedBuffer.size()
			  << ", contents='" << _chunkedBuffer << "'\n";
}

void ParsedRequest::clearChunkedBuffer()
{
	_chunkedBuffer.clear();
}

std::string ParsedRequest::decodeChunkedBody()
{
    std::string decoded;
    size_t pos = 0;

    while (pos < _chunkedBuffer.size())
    {
        // Find next CRLF to read chunk size
        size_t crlfPos = _chunkedBuffer.find("\r\n", pos);
        if (crlfPos == std::string::npos)
            break; // incomplete chunk size line, wait for more data

        std::string sizeStr = _chunkedBuffer.substr(pos, crlfPos - pos);
        size_t chunkSize = std::stoul(sizeStr, nullptr, 16); // may throw invalid_argument or out_of_range

        pos = crlfPos + 2; // move past CRLF

        if (chunkSize == 0)
        {
            setTerminatingZeroChunkReceived();
            pos += 2; // skip final CRLF
            _chunkedBuffer.erase(0, pos);
            return decoded;
        }

        // Check if full chunk data is available
        if (pos + chunkSize > _chunkedBuffer.size())
            break; // incomplete chunk, wait for more data

        // Append chunk to decoded
        decoded += _chunkedBuffer.substr(pos, chunkSize);
        pos += chunkSize;

        // Skip CRLF after chunk
        if (_chunkedBuffer.substr(pos, 2) == "\r\n")
            pos += 2;
        else
            break; // malformed chunk, wait for more data
    }

    // erase fully decoded part of buffer
    _chunkedBuffer.erase(0, pos);

    return decoded; // may be partial if last chunk incomplete
}

bool ParsedRequest::contentLengthComplete() const
{
	if (_contentLength < 0) // defensive: invalid content length
		return false;

	return _contentLengthBuffer.size() >= static_cast<size_t>(_contentLength);
}

const std::string& ParsedRequest::getContentLengthBuffer() const
{
	return _contentLengthBuffer;
}
 
void ParsedRequest::appendToContentLengthBuffer(const std::string& data)
{
	_contentLengthBuffer += data;
}
void ParsedRequest::clearContentLengthBuffer()
{
	_contentLengthBuffer.clear();
}

BodyType ParsedRequest::getBodyType() const
{ 
	return _bodyType;
}
 
void ParsedRequest::setBodyType(BodyType type)
{
	_bodyType = type;
}

void ParsedRequest::setTerminatingZeroChunkReceived()
{
	_terminatingZeroReceived = true;
}

bool ParsedRequest::isTerminatingZeroChunkReceived()
{
	return _terminatingZeroReceived;
}

	
const std::string& ParsedRequest::getTempBuffer() const
{
    return _tempBuffer;
}

void ParsedRequest::setTempBuffer(const std::string& buffer)
{
    _tempBuffer = buffer;
}

void ParsedRequest::appendTempBuffer(const std::string& data)
{
    _tempBuffer += data;
}

void ParsedRequest::clearTempBuffer()
{
    _tempBuffer.clear();
}