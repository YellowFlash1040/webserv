#include "RawRequest.hpp"

RawRequest::RawRequest()
	: _tempBuffer(), _body(), _chunkedBuffer(), _conLenBuffer(), _method(), _uri(), _host(), _httpVersion(),
	_headers(), _bodyType(BodyType::NO_BODY), _headersDone(false), _terminatingZeroMet(false), _bodyDone(false),
	_requestDone(false), _isBadRequest(false), _shouldClose(false) {}

// -----------------------------
// Public Parsing / Data Methods
// -----------------------------

bool RawRequest::parse()
{
	// Parse headers if not done
	if (!isHeadersDone())
	{
		handleHeaderPart();

		if (isBadRequest())
		{
			DBG("[parseRawRequest] Bad request detected");
			setRequestDone();
			return true;
		}

		if (!isHeadersDone())
		{
			DBG("[parseRawRequest]: headers are not finished yet");
			return false; // need more data
		}
	}

	// Parse body if needed
	if (!isBadRequest() && isHeadersDone() && !isBodyDone())
	{
		appendBodyBytes(getTempBuffer());

		if (!isBodyDone())
		{
			DBG("[parseRawRequest]: body not finished yet");
			return false; // need more data
		}
	}

	// Full request done
	if ((isHeadersDone() && isBodyDone()) || isBadRequest())
	{
		DBG("[parseRawRequest]: request done");
		setRequestDone();
		return true;
	}

	return false;
}

RequestData RawRequest::buildRequestData() const
{
	RequestData data;

	data.method = _method;
	data.uri = _uri;
	data.query = _query;
	data.httpVersion = _httpVersion;
	data.headers = _headers;
	data.body = _body;
	return data;
}

bool RawRequest::isRequestDone() const
{ 
	return _requestDone;
}

bool RawRequest::isBadRequest() const
{
	return _isBadRequest;
}

bool RawRequest::shouldClose() const
{ 
	return _shouldClose;
}

HttpMethod RawRequest::getMethod() const
{
	return _method;
}

const std::string& RawRequest::getUri() const
{
	return _uri;
}

const std::string& RawRequest::getQuery() const
{
	return _query;
}

const std::string& RawRequest::getHttpVersion() const 
{
	return _httpVersion;
}

const std::unordered_map<std::string, std::string>& RawRequest::getHeaders() const
{
	return _headers;
}

const std::string RawRequest::getHeader(const std::string& name) const
{
	auto it = _headers.find(name);
	return (it != _headers.end()) ? it->second : "";
}

std::string RawRequest::getHost() const
{
	return _host;
}

BodyType::Type RawRequest::getBodyType() const
{
	return _bodyType;
}

const std::string& RawRequest::getTempBuffer() const
{
	return _tempBuffer;
}

const std::string& RawRequest::getBody() const
{
	return _body;
}

// -----------------------------
// Public Setters / Modifiers
// -----------------------------

void RawRequest::setMethod(HttpMethod method)
{
	_method = method;
}

void RawRequest::setUri(const std::string& uri)
{
	_uri = uri;
}

void RawRequest::setHeadersDone()
{ 
	_headersDone = true;
}

void RawRequest::addHeader(const std::string& name, const std::string& value)
{
	auto it = _headers.find(name);
	if (it != _headers.end())
	{
		// Header already exists
		if (!StrUtils::equalsIgnoreCase(it->second, value))
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

void RawRequest::setShouldClose(bool value)
{
	_shouldClose = value;
}

void RawRequest::setTempBuffer(const std::string& buffer)
{
	_tempBuffer = buffer;
}

void RawRequest::appendTempBuffer(const std::string& data)
{
	_tempBuffer += data;
}

// -----------------------------
// Private Parsing Helpers
// -----------------------------

void RawRequest::handleHeaderPart()
{
	DBG("handleHeaderPart");

	std::string headerPart;
	
	if (!extractHeaderPart(headerPart))
	{
		DBG("Headers incomplete");
		return;
	}
	
	DBG("Header part extracted, length = " << headerPart.size());

	parseRequestLineAndHeaders(headerPart);

	if (_bodyType == BodyType::NO_BODY)
	{
		DBG("No body, marking body done and request done");
		_bodyDone = true;
		_requestDone = true;
	}

	finalizeHeaderPart();
}

bool RawRequest::extractHeaderPart(std::string& headerPart)
{
	size_t headerEnd = _tempBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;

	headerPart = _tempBuffer.substr(0, headerEnd + 4);
	
	// Keep leftover (after headers) in tempBuffer
	_tempBuffer = _tempBuffer.substr(headerEnd + 4);
	DBG("Temp buffer length after header removal = " << _tempBuffer.size());

	return true;
}

void RawRequest::parseRequestLineAndHeaders(const std::string& headerPart)
{
	try
	{
		std::istringstream stream(headerPart);
		std::string line;
		if (!std::getline(stream, line))
			throw std::runtime_error("Malformed request: missing request line");

		StrUtils::removeCarriageReturns(line);

		DBG("[parseRequestLineAndHeaders] Request line: " << line);

		parseRequestLine(line);
		parseHeaders(stream);
	}
	catch(const std::exception& e)
	{
		DBG("[parseRequestLineAndHeaders] Bad request: " << e.what());
		markBadRequest(); // sets _requestDone and prepares 400 response
	}
}

void RawRequest::parseRequestLine(const std::string& firstLine)
{
	std::istringstream reqLine(firstLine);
	std::string methodStr;
	
	reqLine >> methodStr >> _rawUri >> _httpVersion;

	if (methodStr.empty() || _rawUri.empty() || _httpVersion.empty())
		throw std::runtime_error("Invalid request line");

	_method = stringToHttpMethod(methodStr);
	if (_method == HttpMethod::NONE)
		throw std::invalid_argument("Unsupported HTTP method: " + methodStr);

	if (_httpVersion != "HTTP/1.0" && _httpVersion != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + _httpVersion);

	if (_rawUri[0] != '/')
	{
		throw std::invalid_argument("Invalid request URI: " + _rawUri);
		// 400 Bad Request
	}

	splitUriAndQuery();
}

void RawRequest::splitUriAndQuery()
{
	size_t qpos = _rawUri.find('?');
	if (qpos != std::string::npos)
	{
		_uri = _rawUri.substr(0, qpos);
		_query = _rawUri.substr(qpos + 1);
	}
	else
	{
		_uri = _rawUri;
		_query.clear();
	}

	DBG("[splitUriAndQuery]: _uri = " << _uri << ", _query = " << _query);
}

void RawRequest::parseHeaders(std::istringstream& stream)
{
	std::string line;
	while (std::getline(stream, line) && !line.empty() && line != "\r")
	{
		StrUtils::removeCarriageReturns(line);
		parseAndStoreHeaderLine(line);
	}

	// Decide body type and connection behavior
	finalizeHeaders();
}

void RawRequest::parseAndStoreHeaderLine(const std::string& line)
{
	auto colonPos = line.find(':');
	if (colonPos == std::string::npos)
		throw std::invalid_argument("Malformed header line: " + line);

	std::string key = line.substr(0, colonPos);
	std::string value = line.substr(colonPos + 1);
	StrUtils::trimLeadingWhitespace(value);
	try
	{
		addHeader(key, value);
	}
	catch(const std::exception& e)
	{
		throw std::invalid_argument("Header parse error: " + std::string(e.what()));
	}
	if (StrUtils::equalsIgnoreCase(key, "Host"))
	{
		size_t portPos = value.find(':');
		_host = (portPos != std::string::npos) ? value.substr(0, portPos) : value;
	}
}

void RawRequest::finalizeHeaders()
{
	if (StrUtils::equalsIgnoreCase(getHeader("Transfer-Encoding"), "chunked"))
		_bodyType = BodyType::CHUNKED;
	else if (!getHeader("Content-Length").empty())
		_bodyType = BodyType::SIZED;

	if (StrUtils::equalsIgnoreCase(getHeader("Connection"), "close"))
		_shouldClose = true;

	_headersDone = true;
}

void RawRequest::finalizeHeaderPart()
{
	if (_bodyType == BodyType::NO_BODY)
	{
		DBG("No body, marking body done and request done");
		_bodyDone = true;
		_requestDone = true;
	}

	try
	{
		_uri = UriUtils::normalizePath(_uri);
	}
	catch (const std::exception&)
	{
		markBadRequest();
	}
}

void RawRequest::appendBodyBytes(const std::string& data)
{
	try
	{
		switch (_bodyType)
		{
			case BodyType::SIZED:
			{
				// Call BodyParser's version with references
				BodyParser::parseSizedBody(
					data,            // incomingData
					_conLenBuffer,   // conLenBuffer
					_tempBuffer,     // tempBuffer
					getContentLengthValue(), // expectedLength
					_bodyDone        // bodyDone
				);

				if (_bodyDone)
				{
					appendToBody(_conLenBuffer); // only append when done
					DBG("[appendBodyBytes]: Content-Length body finished, body appended, bodyDone set");
				}
				break;
			}

			case BodyType::CHUNKED:
			{
				// Call BodyParser's reference-based parseChunkedBody
				BodyParser::parseChunkedBody(
					_chunkedBuffer,    // chunkedBuffer
					_tempBuffer,       // tempBuffer
					_body,             // body
					_terminatingZeroMet, // terminatingZeroMet
					_bodyDone          // bodyDone
				);
				break;
			}

			case BodyType::NO_BODY:
				DBG("[appendBodyBytes]: No body type, nothing to append");
				break;

			case BodyType::ERROR:
				throw std::runtime_error("Cannot append body data: request in ERROR state");
		}
	}
	catch(const std::exception& e)
	{
		DBG("[appendBodyBytes] Bad request: " << e.what());
		markBadRequest(); // sets _requestDone and prepares 400 response
	}

}

size_t RawRequest::getContentLengthValue() const
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

void RawRequest::appendToBody(const std::string& data)
{
	DBG("[appendToBody]: Appending " << data.size() << " bytes to _body");

	_body += data;

	DBG("[appendToBody]: _body now = |" << _body << "|");
}

bool RawRequest::isHeadersDone() const
{ 
	return _headersDone;
}

bool RawRequest::isBodyDone() const
{ 
	return _bodyDone;
}

void RawRequest::setRequestDone()
{
	_requestDone = true;
}

void RawRequest::markBadRequest()
{
	_isBadRequest = true;
	_headersDone = true;
	_bodyDone = true;
	_requestDone = true;
}