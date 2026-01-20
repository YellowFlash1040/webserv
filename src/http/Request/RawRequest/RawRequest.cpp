#include "RawRequest.hpp"

RawRequest::RawRequest()
	: m_tempBuffer(), m_body(), m_chunkedBuffer(), m_conLenBuffer(), m_method(), m_uri(), m_host(), m_httpVersion(),
	m_headers(), m_bodyType(BodyType::NO_BODY), m_headersDone(false), m_terminatingZeroMet(false), m_bodyDone(false),
	m_requestDone(false), m_isBadRequest(false), m_shouldClose(false) {}

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

	data.method = m_method;
	data.uri = m_uri;
	data.query = m_query;
	data.httpVersion = m_httpVersion;
	data.headers = m_headers;
	data.body = m_body;
	return data;
}

bool RawRequest::isRequestDone() const
{ 
	return m_requestDone;
}

bool RawRequest::isBadRequest() const
{
	return m_isBadRequest;
}

bool RawRequest::shouldClose() const
{ 
	return m_shouldClose;
}

HttpMethod RawRequest::getMethod() const
{
	return m_method;
}

const std::string& RawRequest::getUri() const
{
	return m_uri;
}

const std::string& RawRequest::getQuery() const
{
	return m_query;
}

const std::string& RawRequest::getHttpVersion() const 
{
	return m_httpVersion;
}

const std::unordered_map<std::string, std::string>& RawRequest::getHeaders() const
{
	return m_headers;
}

const std::string RawRequest::getHeader(const std::string& name) const
{
	auto it = m_headers.find(name);
	return (it != m_headers.end()) ? it->second : "";
}

std::string RawRequest::getHost() const
{
	return m_host;
}

BodyType RawRequest::getBodyType() const
{
	return m_bodyType;
}

const std::string& RawRequest::getTempBuffer() const
{
	return m_tempBuffer;
}

const std::string& RawRequest::getBody() const
{
	return m_body;
}

// -----------------------------
// Public Setters / Modifiers
// -----------------------------

void RawRequest::setMethod(HttpMethod method)
{
	m_method = method;
}

void RawRequest::setUri(const std::string& uri)
{
	m_uri = uri;
}

void RawRequest::setHeadersDone()
{ 
	m_headersDone = true;
}

void RawRequest::addHeader(const std::string& name, const std::string& value)
{
	auto it = m_headers.find(name);
	if (it != m_headers.end())
	{
		// Header already exists
		if (!StrUtils::equalsIgnoreCase(it->second, value))
		{
			// Conflict: same header with different values
			m_bodyType = BodyType::ERROR;
			throw std::runtime_error("Header conflict: " + name +
									 " has values [" + it->second + "] and [" + value + "]");
		}
		else
		{
			// Duplicate with same value: harmless
			return;
		}
	}
	m_headers[name] = value;

}

void RawRequest::setShouldClose(bool value)
{
	m_shouldClose = value;
}

void RawRequest::setTempBuffer(const std::string& buffer)
{
	m_tempBuffer = buffer;
}

void RawRequest::setBody(const std::string& data)
{
	m_body = data;
}

void RawRequest::appendTempBuffer(const std::string& data)
{
	m_tempBuffer += data;
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

	if (m_bodyType == BodyType::NO_BODY)
	{
		DBG("No body, marking body done and request done");
		m_bodyDone = true;
		m_requestDone = true;
	}

	finalizeHeaderPart();
}

bool RawRequest::extractHeaderPart(std::string& headerPart)
{
	size_t headerEnd = m_tempBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;

	headerPart = m_tempBuffer.substr(0, headerEnd + 4);
	
	// Keep leftover (after headers) in tempBuffer
	m_tempBuffer = m_tempBuffer.substr(headerEnd + 4);
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
	
	reqLine >> methodStr >> m_rawUri >> m_httpVersion;

	if (methodStr.empty() || m_rawUri.empty() || m_httpVersion.empty())
		throw std::runtime_error("Invalid request line");

	m_method = stringToHttpMethod(methodStr);
	if (m_method == HttpMethod::NONE)
		throw std::invalid_argument("Unsupported HTTP method: " + methodStr);

	if (m_httpVersion != "HTTP/1.0" && m_httpVersion != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + m_httpVersion);

	if (m_rawUri[0] != '/')
	{
		throw std::invalid_argument("Invalid request URI: " + m_rawUri);
	}

	splitUriAndQuery();
}

void RawRequest::splitUriAndQuery()
{
	size_t qpos = m_rawUri.find('?');
	if (qpos != std::string::npos)
	{
		m_uri = m_rawUri.substr(0, qpos);
		m_query = m_rawUri.substr(qpos + 1);
	}
	else
	{
		m_uri = m_rawUri;
		m_query.clear();
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
		m_host = (portPos != std::string::npos) ? value.substr(0, portPos) : value;
	}
}

void RawRequest::finalizeHeaders()
{
	if (StrUtils::equalsIgnoreCase(getHeader("Transfer-Encoding"), "chunked"))
		m_bodyType = BodyType::CHUNKED;
	else if (!getHeader("Content-Length").empty())
		m_bodyType = BodyType::SIZED;

	if (StrUtils::equalsIgnoreCase(getHeader("Connection"), "close"))
		m_shouldClose = true;

	m_headersDone = true;
}

void RawRequest::finalizeHeaderPart()
{
	try
	{
		m_uri = UriUtils::normalizePath(m_uri);
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
		switch (m_bodyType)
		{
			case BodyType::SIZED:
			{
				BodyParser::parseSizedBody(
					data,
					m_conLenBuffer,
					m_tempBuffer,
					getContentLengthValue(),
					m_bodyDone
				);

				if (m_bodyDone)
				{
					appendToBody(m_conLenBuffer); // only append when done
					DBG("[appendBodyBytes]: Content-Length body finished, body appended, bodyDone set");
				}
				break;
			}

			case BodyType::CHUNKED:
			{
				BodyParser::parseChunkedBody(
					m_chunkedBuffer,
					m_tempBuffer,
					m_body,
					m_terminatingZeroMet,
					m_bodyDone
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
	auto clIt = m_headers.find("Content-Length");
	if (clIt == m_headers.end())
		return 0;

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

	m_body += data;

	DBG("[appendToBody]: _body now = |" << _body << "|");
}

bool RawRequest::isHeadersDone() const
{ 
	return m_headersDone;
}

bool RawRequest::isBodyDone() const
{ 
	return m_bodyDone;
}

void RawRequest::setRequestDone()
{
	m_requestDone = true;
}

void RawRequest::markBadRequest()
{
	m_isBadRequest = true;
	m_headersDone = true;
	m_bodyDone = true;
	m_requestDone = true;
}