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
	_headers(), _bodyType(BodyType::NO_BODY), _body(), _headersDone(false), _terminatingZero(false), _bodyDone(false),
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

std::string& ParsedRequest::getContentLengthBuffer()
{
	return _contentLengthBuffer;
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

void ParsedRequest::parseRequestLineAndHeaders(const std::string& headerPart)
{
	std::istringstream stream(headerPart);
	std::string line;
	if (!std::getline(stream, line))
		throw std::runtime_error("Malformed request: missing request line");
		
	removeCarriageReturns(line);

	parseRequestLine(line); // throws invalid_argument if broken
	parseHeaders(stream); // throws invalid_argument if broken

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
			throw std::invalid_argument("Malformed header line: " + line);

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		trimLeadingWhitespace(value);
		try
		{
			addHeader(key, value);
		}
		catch(const std::exception& e)
		{
			throw std::invalid_argument("Header parse error: " + std::string(e.what()));
		}
	}

	// Decide body type
	if (iequals(getHeader("Transfer-Encoding"), "chunked"))
	{
		setBodyType(BodyType::CHUNKED);
	}
	else if (!getHeader("Content-Length").empty())
	{
		_contentLength = extractContentLength();
		setBodyType(BodyType::SIZED);
	} 

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

std::string& ParsedRequest::getChunkedBuffer()
{
	return _chunkedBuffer;
}

void ParsedRequest::appendToChunkedBuffer(const std::string& data)
{
	std::cout << "\033[33mDEBUG: appendToChunkedBuffer\033[0m" << std::endl;
	std::cout << ORANGE << "[appendToChunkedBuffer]: " << RESET << "Before append, _chunkedBuffer size=" << _chunkedBuffer.size() << "\n";

	_chunkedBuffer += data;

	std::cout << ORANGE << "[appendToChunkedBuffer]: " << RESET << "After append, _chunkedBuffer size=" << _chunkedBuffer.size()
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
		
    std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "START, buffer='" 
              << _chunkedBuffer << "' (size=" << _chunkedBuffer.size() << "\n";

    while (pos < _chunkedBuffer.size())
    {
        std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "pos=" << pos 
                  << ", buffer from here='" << _chunkedBuffer.substr(pos) << "'\n";

        // Find next CRLF to read chunk size
        size_t crlfPos = _chunkedBuffer.find("\r\n", pos);
        if (crlfPos == std::string::npos)
		{
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "No CRLF found, incomplete chunk size\033[0m\n";
            break; // incomplete chunk size line, wait for more data
        }

        std::string sizeStr = _chunkedBuffer.substr(pos, crlfPos - pos);
        size_t chunkSize = std::stoul(sizeStr, nullptr, 16); // may throw

        std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "Found chunk size='" << sizeStr 
                  << "' (" << chunkSize << " bytes)\n";

        pos = crlfPos + 2; // move past CRLF

        if (chunkSize == 0)
        {
            std::cout << ORANGE << "[decodeChunkedBody]:" << RESET << "Zero-size chunk found, terminating body\n";
            setTerminatingZero();
            pos += 2; // skip final CRLF
            _chunkedBuffer.erase(0, pos);
            std::cout << ORANGE << "[decodeChunkedBody] " << RESET << "After erase, buffer='" 
                      << _chunkedBuffer << "'";
            return decoded;
        }

        // Check if full chunk data is available
        if (pos + chunkSize > _chunkedBuffer.size())
		{
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "Incomplete chunk data, need " 
                      << chunkSize << " bytes but only " 
                      << (_chunkedBuffer.size() - pos) << " available\n";
            break; // incomplete chunk, wait for more data
        }

        // Append chunk to decoded
        std::string chunkData = _chunkedBuffer.substr(pos, chunkSize);
        decoded += chunkData;

        std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "Appended chunk='" << chunkData 
                  << "', decoded now='" << decoded << "'\n";

        pos += chunkSize;

        // Skip CRLF after chunk
        if (_chunkedBuffer.substr(pos, 2) == "\r\n")
		{
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "Skipping CRLF after chunk\n";
            pos += 2;
        }
		else
		{
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "Missing CRLF after chunk, breaking\n";
            break; // malformed chunk, wait for more data
        }
    }

    // erase fully decoded part of buffer
    if (pos > 0)
	{
        std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "Erasing " << pos 
                  << " bytes from buffer\n";
        _chunkedBuffer.erase(0, pos);
    }

    std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << "END, decoded='" << decoded 
              << "', buffer now='" << _chunkedBuffer << "'\n";

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

void ParsedRequest::setTerminatingZero()
{
	_terminatingZero = true;
}

bool ParsedRequest::isTerminatingZero()
{
	return _terminatingZero;
}

	
std::string& ParsedRequest::getTempBuffer()
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