#include "RawRequest.hpp"

RawRequest::RawRequest()
	: _tempBuffer(), _rlAndHeadersBuffer(), _body(), _chunkedBuffer(), _conLenBuffer(), _method(), _uri(), _httpVersion(),
	_headers(), _bodyType(BodyType::NO_BODY), _errorMessage(), _headersDone(false), _terminatingZeroMet(false), _bodyDone(false),
	_requestDone(false), _isBadRequest(false), _shouldClose(false) {}

bool RawRequest::parse()
{
	// Parse headers if not done
	if (!isHeadersDone())
	{
		separateHeadersFromBody();

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
		markBadRequest(e.what()); // sets _requestDone and prepares 400 response
	}
}
	
void RawRequest::parseHeaders(std::istringstream& stream)
{
	std::string line;
	while (std::getline(stream, line) && !line.empty() && line != "\r")
	{
		StrUtils::removeCarriageReturns(line);
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
	}

	// Decide body type
	if (StrUtils::equalsIgnoreCase(getHeader("Transfer-Encoding"), "chunked"))
	{
		_bodyType = BodyType::CHUNKED;
	}
	else if (!getHeader("Content-Length").empty())
	{
		_bodyType = BodyType::SIZED;
	}
	
	if (StrUtils::equalsIgnoreCase(getHeader("Connection"), "close"))
	{
		_shouldClose = true;
	}

	_headersDone = true;
}

void RawRequest::appendToChunkedBuffer(const std::string& data)
{
	DBG("\n[appendToChunkedBuffer]\nBefore append, _chunkedBuffer size = " << _chunkedBuffer.size());

	_chunkedBuffer += data;

	DBG("[appendToChunkedBuffer] After append, _chunkedBuffer size = " << _chunkedBuffer.size());
}

std::string RawRequest::decodeChunkedBody(size_t& bytesProcessed)
{
	DBG("[decodeChunkedBody]:"
		<< " START: _chunkedBuffer = |" << _chunkedBuffer 
		<< "|, _chunkedBuffer size = " << _chunkedBuffer.size());

	std::string decoded;
	size_t pos = 0;
	bytesProcessed = 0;

	while (pos < _chunkedBuffer.size())
	{
		// Find end of the current chunk header line
		size_t chunkLineEnd = _chunkedBuffer.find("\r\n", pos);
		if (chunkLineEnd == std::string::npos)
		{
			DBG("[decodeChunkedBody]: Incomplete chunkHeaderLine, waiting for more data");
			break; // wait for more data
		}

		std::string chunkHeaderLine = _chunkedBuffer.substr(pos, chunkLineEnd - pos);
		DBG("[decodeChunkedBody]: chunkHeaderLine is |" << chunkHeaderLine);
		
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
			DBG("[decodeChunkedBody]: Invalid chunk size |" << chunkSizeStr << "|, throwing exception");
			throw std::runtime_error("Invalid chunk size in chunked body");
		}

		DBG("[decodeChunkedBody]: Found the chunkHeaderLine |" << chunkHeaderLine 
			<< "|, so chunkSize = " << chunkSize << " bytes");

		size_t chunkDataStart = chunkLineEnd + 2; // skip \r\n
		DBG("[decodeChunkedBody]: Skipped \\r\\n from chunkLineEnd at pos " << chunkLineEnd << ". chunkDataStart pos is " << chunkDataStart);
		
		size_t chunkDataEnd = chunkDataStart + chunkSize;
		DBG("[decodeChunkedBody]: chunkDataEnd pos is " << chunkDataEnd);

		if (chunkDataEnd > _chunkedBuffer.size())
		{
			DBG("[decodeChunkedBody]: Incomplete chunkData, waiting for more data");
			break; // wait for more data
		}

		// Append chunk data
		if (chunkSize > 0)
		{
			// Append chunk data
			std::string chunkData = _chunkedBuffer.substr(chunkDataStart, chunkSize);
			decoded += chunkData;
			DBG("[decodeChunkedBody]: Appended chunkData |" << chunkData 
				<< "|, to decoded. decoded is now |" << decoded
				<< "|, decoded.size = " << decoded.size());
			
				pos = chunkDataEnd + 2; // skip chunkData + trailing \r\n
				DBG("[decodeChunkedBody]: skipped chunkData + chunkTrailer, pos is " << pos);
			}
		else
		{
			// terminating zero chunk, make sure final CRLF exists
			DBG("[decodeChunkedBody]: reached terminating zero chunk!!");

			// make sure the final CRLF exists
			size_t zeroChunkEnd = chunkDataStart + 2; // chunkDataStart points after the first \r\n
			DBG("[decodeChunkedBody]: zeroChunkEnd is set to 2 bytes after chunkDataStart (" << chunkDataStart
				<< "), so at " << zeroChunkEnd);
			

			if (_chunkedBuffer.size() >= zeroChunkEnd
				&& _chunkedBuffer[chunkDataStart] == '\r'
				&& _chunkedBuffer[chunkDataStart + 1] == '\n')
			{
				pos = zeroChunkEnd;
				_terminatingZeroMet = true;
				DBG("[decodeChunkedBody]: Zero-size chunk fully consumed, pos = " << pos);
				break;
			}
			else
			{
				DBG("[decodeChunkedBody]: waiting for final CRLF after zero chunk");
				break; // wait for more data
			}
			break; // zero chunk ends the loop
		}
	}
	
	bytesProcessed = pos;
	DBG("[decodeChunkedBody]: END: decoded is |" << decoded << "| with size = " << decoded.size()
		<< ", bytesProcessed = " << bytesProcessed);

	return decoded;
}



void RawRequest::appendToConLenBuffer(const std::string& data)
{
	_conLenBuffer += data;
}

void RawRequest::setTempBuffer(const std::string& buffer)
{
	_tempBuffer = buffer;
}

void RawRequest::appendTempBuffer(const std::string& data)
{
	_tempBuffer += data;
}

size_t RawRequest::remainingConLen() const
{
	return static_cast<size_t>(getContentLengthValue()) > _conLenBuffer.size()
		? static_cast<size_t>(getContentLengthValue()) - _conLenBuffer.size()
		: 0;
}

void RawRequest::consumeTempBuffer(size_t n)
{
	DBG("[consumeTempBuffer]: _tempBuffer before: |" << _tempBuffer 
		<< "|, size = " << _tempBuffer.size());

	if (n >= _tempBuffer.size())
	{
		_tempBuffer.clear();
	}
	else
	{
		_tempBuffer.erase(0, n);  // remove the first n bytes
	}

	DBG("[consumeTempBuffer]: after consuming: |" << _tempBuffer 
		<< "|, size = " << _tempBuffer.size());
}

void RawRequest::appendToBody(const std::string& data)
{
	DBG("[appendToBody]: Appending " << data.size() << " bytes to _body");

	_body += data;

	DBG("[appendToBody]: _body now = |" << _body << "|");
}

void RawRequest::setChunkedBuffer(std::string&& newBuffer)
{
	DBG("[setChunkedBuffer]: old size of _chunkedBuffer = " << _chunkedBuffer.size());

	_chunkedBuffer = std::move(newBuffer);

	DBG("[setChunkedBuffer]: new size of _chunkedBuffer = " << _chunkedBuffer.size()
		<< ", content = |" << _chunkedBuffer << "|");
}
void RawRequest::parseSizedBody(const std::string& data)
{
	size_t remaining = remainingConLen(); // bytes still needed
	DBG("[appendBodyBytes]: remaining bytes of content to append = " << remaining);
	
	size_t toAppend = std::min(remaining, data.size());
	DBG("[appendBodyBytes]: bytes to append in reality = " << toAppend);
	
	appendToConLenBuffer(data.substr(0, toAppend));
	consumeTempBuffer(toAppend); // remove exactly what we consumed
	
	if (conLenReached())
	{
		appendToBody(_conLenBuffer);
		setBodyDone();
		DBG("[appendBodyBytes]: Content-Length body finished, body done set");
	}
}

void RawRequest::parseChunkedBody()
{

	appendToChunkedBuffer(_tempBuffer);
	DBG("[appendBodyBytes]: appended _chunkedBuffer with data from _tempBuffer, _chunkedBuffer size = " 
		<< _chunkedBuffer.size());
	
	setTempBuffer(""); // consumed for decoding
	size_t bytesProcessed = 0;
	
	// decode as much as possible
	std::string decoded = decodeChunkedBody(bytesProcessed);
	appendToBody(decoded); // append only the decoded chunks
	DBG("[appendBodyBytes]: decoded chunked body appended, decoded size = " << decoded.size());
	
	// remove processed bytes from _chunkedBuffer
	setChunkedBuffer(_chunkedBuffer.substr(bytesProcessed));
	DBG("[appendBodyBytes]: _chunkedBuffer resized after processing, new size = " << _chunkedBuffer.size());
	
	if (_terminatingZeroMet)
	{
		setBodyDone();
		setTempBuffer(_chunkedBuffer);
		_chunkedBuffer.clear();
		DBG("[appendBodyBytes]: Terminating zero chunk found, body done set, tempBuffer updated, _chunkedBuffer cleared");
	}
	else
	{
		// partial chunk left? move leftovers to tempBuffer for next process
		setTempBuffer(_chunkedBuffer + _tempBuffer);
		
		DBG("[appendBodyBytes]: Terminating zero not found, leftovers moved to tempBuffer");
	}
}

void RawRequest::appendBodyBytes(const std::string& data)
{

	switch (_bodyType)
	{
		case BodyType::SIZED:
		{
			parseSizedBody(data);
			break;
		}

		case BodyType::CHUNKED:
		{
			parseChunkedBody();
			break;
		}

		case BodyType::NO_BODY:
			// Nothing to append
			DBG("[appendBodyBytes]: No body type, nothing to append");
			break;

		case BodyType::ERROR:
			throw std::runtime_error("Cannot append body data: request in ERROR state");
	}
	DBG("[appendBodyBytes]: END");
	
}

void RawRequest::separateHeadersFromBody()
{
	DBG("separateHeadersFromBody");

	size_t headerEnd = _tempBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		DBG("Headers incomplete (\\r\\n\\r\\n not found)");
		return; // headers incomplete
	}

	std::string headerPart = _tempBuffer.substr(0, headerEnd + 4);
	DBG("Header part extracted, length = " << headerPart.size());

	parseRequestLineAndHeaders(headerPart);

	if (_bodyType == BodyType::NO_BODY)
	{
		DBG("No body, marking body done and request done");
		_bodyDone = true;
		_requestDone = true;
	}

	// Keep leftover (after headers) in tempBuffer
	_tempBuffer = _tempBuffer.substr(headerEnd + 4);
	DBG("Temp buffer length after header removal = " << _tempBuffer.size());

	try
	{
		_uri =  UriUtils::normalizePath(_uri);  // validate path AFTER leftovers are handled
	}
	catch (const std::exception& e)
	{
		markBadRequest(e.what());
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
		throw std::invalid_argument("Unsupported HTTP method: " + httpMethodToString(_method));

	if (_httpVersion != "HTTP/1.0" && _httpVersion != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + _httpVersion);

	if (_rawUri[0] != '/')
	{
		throw std::invalid_argument("Invalid request URI: " + _rawUri);
		// 400 Bad Request
	}

	// --- Split URI into path + query ---
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

	DBG("[parseRequestLine]: _uri is " << _uri);
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

void RawRequest::markBadRequest(const std::string& msg = "")
{
	_isBadRequest = true;
	_headersDone = true;
	_bodyDone = true;
	_requestDone = true;
	_errorMessage = msg;
}

void RawRequest::printRequest(size_t idx) const
{
	(void)idx;
	DBG("\n[RawRequest]"
		<< "\nMethod: " << httpMethodToString(_method)
		<< "\nURI: " << _uri
		<< "\nQuery: " << _query
		<< "\nHTTP Version: " << _httpVersion
		<< "\nBody Type: " << BodyType::toString(_bodyType)
		<< "\nBody Length: " << _body.size()
		<< "\nHeaders Done: " << (_headersDone ? "true" : "false")
		<< "\nBody Done: " << (_bodyDone ? "true" : "false")
		<< "\nRequest Done: " << (_requestDone ? "true" : "false"));
		
		for (const auto& header : _headers)
		{
			DBG("  " << header.first << ": " << header.second);
			(void)header;
		}
}

bool RawRequest::isBadRequest() const
{
	return _isBadRequest;
}

bool RawRequest::conLenReached() const
{
	return _conLenBuffer.size() >= static_cast<size_t>(getContentLengthValue());
}

bool RawRequest::isRequestDone() const
{ 
	return _requestDone;
}

bool RawRequest::isHeadersDone() const
{ 
	return _headersDone;
}

bool RawRequest::isBodyDone() const
{ 
	return _bodyDone;
}

bool RawRequest::shouldClose() const
{ 
	return _shouldClose;
}



HttpMethod RawRequest::getMethod() const
{
	return _method;
}

const std::string& RawRequest::getBody() const
{
    return _body;
}

const std::string& RawRequest::getHttpVersion() const 
{
    return _httpVersion;
}

const std::string& RawRequest::getUri() const
{
	return _uri;
}

std::string RawRequest::getHost() const
{
	auto it = _headers.find("Host");
	return (it != _headers.end()) ? it->second : "";
}

const std::unordered_map<std::string, std::string>& RawRequest::getHeaders() const
{
    return _headers;
}

BodyType::Type RawRequest::getBodyType() const
{
    return _bodyType;
}

const std::string RawRequest::getHeader(const std::string& name) const
{
	auto it = _headers.find(name);
	return (it != _headers.end()) ? it->second : "";
}

const std::string& RawRequest::getTempBuffer() const
{
	return _tempBuffer;
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





void RawRequest::setUri(const std::string& uri)
{
	_uri = uri;
}

void RawRequest::setGetMethod() 
{
	_method = HttpMethod::GET;
}

void RawRequest::setMethod(HttpMethod method)
{
	_method = method;
}

void RawRequest::setShouldClose(bool value)
{
	_shouldClose = value;
}

void RawRequest::setHeadersDone()
{ 
	_headersDone = true;
}

void RawRequest::setBodyDone()
{
	_bodyDone = true;
}

void RawRequest::setRequestDone()
{
	_requestDone = true;
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


