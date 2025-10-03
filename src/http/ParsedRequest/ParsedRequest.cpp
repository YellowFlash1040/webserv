#include "ParsedRequest.hpp"

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
	: _tempBuffer(), _rlAndHeadersBuffer(), _body(), _chunkedBuffer(), _method(), _uri(), _httpVersion(),
	_headers(), _bodyType(BodyType::NO_BODY), _headersDone(false), _terminatingZero(false), _bodyDone(false),
	_needResp(true), _requestDone(false), _contentLength(0), _chunked(false) {}

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

const std::string& ParsedRequest::getBody() const
{
	return _body;
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


void ParsedRequest::setRlAndHeadersBuffer(const std::string& newBuf) { _rlAndHeadersBuffer = newBuf; }

// --- Buffer manipulation ---
void ParsedRequest::appendToRlAndHeaderBuffer(const std::string& data) { _rlAndHeadersBuffer += data; }
void ParsedRequest::appendTobody(const std::string& data) { _body += data; }

void ParsedRequest::clearRlAndHeaderBuffer() { _rlAndHeadersBuffer.clear(); }

void ParsedRequest::clearbody() { _body.clear(); }


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


std::string& ParsedRequest::getChunkedBuffer()
{
	return _chunkedBuffer;
}

void ParsedRequest::appendToChunkedBuffer(const std::string& data)
{
	std::cout << YELLOW << "DEBUG: appendToChunkedBuffer" << RESET << std::endl;
	std::cout << ORANGE << "[appendToChunkedBuffer]: " << RESET << "Before append, _chunkedBuffer size = " << _chunkedBuffer.size() << "\n";

	_chunkedBuffer += data;

	std::cout << ORANGE << "[appendToChunkedBuffer]: " << RESET << "After append, _chunkedBuffer size = " << _chunkedBuffer.size()
			  << ", contents = |" << _chunkedBuffer << "|\n";
}

void ParsedRequest::clearChunkedBuffer()
{
	_chunkedBuffer.clear();
}

std::string ParsedRequest::decodeChunkedBody(size_t& bytesProcessed)
{
	std::cout << ORANGE << "[decodeChunkedBody]:" << RESET
		<< " START: _chunkedBuffer = |" << _chunkedBuffer 
		<< "|, _chunkedBuffer size = " << _chunkedBuffer.size() << "\n";

	std::string decoded;
	size_t pos = 0;
	bytesProcessed = 0;

	while (pos < _chunkedBuffer.size())
	{
		// Find end of the current chunk header line
		size_t chunkLineEnd = _chunkedBuffer.find("\r\n", pos);
		if (chunkLineEnd == std::string::npos)
		{
			std::cout << ORANGE << "[decodeChunkedBody]" << RESET
				<< ": Incomplete chunkHeaderLine, waiting for more data\n";
			break; // wait for more data
		}

		std::string chunkHeaderLine = _chunkedBuffer.substr(pos, chunkLineEnd - pos);
		std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
				<< "chunkHeaderLine is |" << chunkHeaderLine << "|\n";
		
		// Extract chunk size (if there are extensions ignore them)
		size_t semicolonPos = chunkHeaderLine.find(';');
		std::string chunkSizeStr = semicolonPos == std::string::npos 
			? chunkHeaderLine : chunkHeaderLine.substr(0, semicolonPos);
		size_t chunkSize = 0;
		try
		{
			chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
		}
		catch (...)
		{
			std::cout << ORANGE << "[decodeChunkedBody]" << RESET
				<< ": Invalid chunk size |" << chunkSizeStr << "|, throwing exception\n";
			throw std::runtime_error("Invalid chunk size in chunked body");
		}

		std::cout << ORANGE << "[decodeChunkedBody]:" << RESET
			<< " Found the chunkHeaderLine |" << chunkHeaderLine 
			<< "|, so chunkSize = " << chunkSize << " bytes\n";

		size_t chunkDataStart = chunkLineEnd + 2; // skip \r\n
		std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
				<< "Skipped \\r\\n from chunkLineEnd at pos " << chunkLineEnd << ". chunkDataStart pos is " << chunkDataStart << "\n"; 
		
		size_t chunkDataEnd = chunkDataStart + chunkSize;
		std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
				<< "chunkDataEnd pos is " << chunkDataEnd << "\n"; 

		if (chunkDataEnd > _chunkedBuffer.size())
		{
			std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
				<< ": Incomplete chunkData, waiting for more data\n";
			break; // wait for more data
		}

		// Append chunk data
		if (chunkSize > 0)
		{
			// Append chunk data
			std::string chunkData = _chunkedBuffer.substr(chunkDataStart, chunkSize);
			decoded += chunkData;
			std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
				<< "Appended chunkData |" << chunkData 
				<< "|, to decoded. decoded is now |" << decoded
				<< "|, decoded.size = " << decoded.size() << "\n";
			
				pos = chunkDataEnd + 2; // skip chunkData + trailing \r\n
				std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
				<< "skipped chunkData + chunkTrailer, pos is " << pos << "\n";
			}
		else
		{
			// terminating zero chunk, make sure final CRLF exists
			std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
            	<< RED << "reached terminating zero chunk" << RESET "\n";

			// make sure the final CRLF exists
			size_t zeroChunkEnd = chunkDataStart + 2; // chunkDataStart points after the first \r\n
			std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
				<< "zeroChunkEnd is set to 2 bytes after chunkDataStart (" << chunkDataStart
				<< "), so at " << zeroChunkEnd << "\n";
			
			// 	_chunkedBuffer[chunkDataStart] == '\r' &&
			// 	_chunkedBuffer[chunkDataStart + 1] == '\n')
			if (_chunkedBuffer.size() >= zeroChunkEnd
				&& _chunkedBuffer[chunkDataStart] == '\r'
				&& _chunkedBuffer[chunkDataStart + 1] == '\n')
			{
				pos = zeroChunkEnd;
				setTerminatingZero();
				std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
						<< "Zero-size chunk fully consumed, pos = " << pos << "\n";
				break;
			}
			else
        	{
				std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
					<< "waiting for final CRLF after zero chunk\n";
                break; // wait for more data
        	}
			break; // zero chunk ends the loop
		}
	}
	
	bytesProcessed = pos;
	std::cout << ORANGE << "[decodeChunkedBody]:" << RESET
		<< " END: decoded is |" << decoded << "| with size = " << decoded.size() 
		<< ", bytesProcessed = " << bytesProcessed << "\n";

	return decoded;
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



bool ParsedRequest::needsResponse() const
{
	return _needResp;
}

void ParsedRequest::setNeedsResp(bool needsResp)
{
	_needResp = needsResp;
}

void ParsedRequest::setResponseAdded()
{
	_needResp = false;
}

void ParsedRequest::setBody(const std::string& body)
{
	_body = body;
}

size_t ParsedRequest::getRemainingContentLength() const
{
	return static_cast<size_t>(_contentLength) > _contentLengthBuffer.size()
		? static_cast<size_t>(_contentLength) - _contentLengthBuffer.size()
		: 0;
}

void ParsedRequest::consumeTempBuffer(size_t n)
{
	std::cout << MINT << "[consumeTempBuffer]: " << RESET << "_tempbuffer before: |" << _tempBuffer << "|, size = " << _tempBuffer.size() << "\n";

    if (n >= _tempBuffer.size())
    {
        _tempBuffer.clear();
    }
    else
    {
        _tempBuffer.erase(0, n);  // remove the first n bytes
    }

    std::cout << MINT << "[consumeTempBuffer]: " << RESET << "after consuming:  |" << _tempBuffer << "|, size = " << _tempBuffer.size() << "\n";
}

void ParsedRequest::appendToBody(const std::string& data)
{
	std::cout << ORANGE << "[appendToBody]: " << RESET
			  << "Appending " << data.size() << " bytes to _body\n";

	_body += data;

	std::cout << ORANGE << "[appendToBody]: " << RESET
			  << "_body now = |" << _body << "|\n";
}

void ParsedRequest::setChunkedBuffer(std::string&& newBuffer)
{
	std::cout << ORANGE << "[setChunkedBuffer]: " << RESET << "old size of chunkedBuffer = " << _chunkedBuffer.size() << "\n";
	_chunkedBuffer = std::move(newBuffer);
	std::cout << "new size = " << _chunkedBuffer.size()
	<< "(moved). content = |" << _chunkedBuffer 
	<< RESET << "|\n";
}

