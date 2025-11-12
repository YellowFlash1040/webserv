#include "RawRequest.hpp"

// --- Helpers ---
static void removeCarriageReturns(std::string& str)
{
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
}

static void trimLeadingWhitespace(std::string& str)
{
    str.erase(str.begin(),
              std::find_if(str.begin(), str.end(),
                           [](unsigned char ch) { return !std::isspace(ch); }));
}

static bool iequals(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::tolower(a[i]) != std::tolower(b[i]))
            return false;
    return true;
}

bool isHex(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')
           || (c >= 'a' && c <= 'f');
}

// --- Canonical form ---
RawRequest::RawRequest()
  : _tempBuffer()
  , _rlAndHeadersBuffer()
  , _body()
  , _chunkedBuffer()
  , _conLenBuffer()
  , _method()
  , _uri()
  , _httpVersion()
  , _headers()
  , _bodyType(BodyType::NO_BODY)
  , _errorMessage()
  , _headersDone(false)
  , _terminatingZeroMet(false)
  , _bodyDone(false)
  , _requestDone(false)
  , _isBadRequest(false)
{
}

RawRequest::~RawRequest() = default;

RawRequest::RawRequest(const RawRequest& other) = default;
RawRequest& RawRequest::operator=(const RawRequest& other) = default;

RawRequest::RawRequest(RawRequest&& other) noexcept = default;
RawRequest& RawRequest::operator=(RawRequest&& other) noexcept = default;

const std::string RawRequest::getHeader(const std::string& name) const
{
    auto it = _headers.find(name);
    return (it != _headers.end()) ? it->second : "";
}

const std::string& RawRequest::getTempBuffer() const
{
    return _tempBuffer;
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

void RawRequest::addHeader(const std::string& name, const std::string& value)
{
    auto it = _headers.find(name);
    if (it != _headers.end())
    {
        // Header already exists
        if (!iequals(it->second, value))
        {
            // Conflict: same header with different values
            _bodyType = BodyType::ERROR;
            throw std::runtime_error("Header conflict: " + name
                                     + " has values [" + it->second + "] and ["
                                     + value + "]");
        }
        else
        {
            // Duplicate with same value: harmless
            return;
        }
    }
    _headers[name] = value;
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

size_t RawRequest::conLenValue() const
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
        throw std::invalid_argument("Invalid Content-Length header: "
                                    + clIt->second);
    }
}

std::string RawRequest::bodyTypeToString(BodyType t)
{
    switch (t)
    {
    case BodyType::NO_BODY:
        return "NO_BODY";
    case BodyType::SIZED:
        return "SIZED";
    case BodyType::CHUNKED:
        return "CHUNKED";
    default:
        return "UNKNOWN";
    }
}

void RawRequest::parseRequestLineAndHeaders(const std::string& headerPart)
{
    try
    {
        std::istringstream stream(headerPart);
        std::string line;
        if (!std::getline(stream, line))
            throw std::runtime_error("Malformed request: missing request line");

        removeCarriageReturns(line);

        parseRequestLine(line);
        parseHeaders(stream);
    }
    catch (const std::exception& e)
    {
        std::cout << "[parseRequestLineAndHeaders] Bad request: " << e.what()
                  << "\n";
        markBadRequest(e.what()); // sets _requestDone and prepares 400 response
    }
}

void RawRequest::parseHeaders(std::istringstream& stream)
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
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Header parse error: "
                                        + std::string(e.what()));
        }
    }

    // Decide body type
    if (iequals(getHeader("Transfer-Encoding"), "chunked"))
    {
        _bodyType = BodyType::CHUNKED;
    }
    else if (!getHeader("Content-Length").empty())
    {
        _bodyType = BodyType::SIZED;
    }

    _headersDone = true;
}

void RawRequest::appendToChunkedBuffer(const std::string& data)
{
    std::cout << YELLOW << "DEBUG: appendToChunkedBuffer" << RESET << std::endl;
    std::cout << ORANGE << "[appendToChunkedBuffer]: " << RESET
              << "Before append, _chunkedBuffer size = "
              << _chunkedBuffer.size() << "\n";

    _chunkedBuffer += data;

    std::cout << ORANGE << "[appendToChunkedBuffer]: " << RESET
              << "After append, _chunkedBuffer size = " << _chunkedBuffer.size()
              << ", contents = |" << _chunkedBuffer << "|\n";
}

std::string RawRequest::decodeChunkedBody(size_t& bytesProcessed)
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
            std::cout
                << ORANGE << "[decodeChunkedBody]" << RESET
                << ": Incomplete chunkHeaderLine, waiting for more data\n";
            break; // wait for more data
        }

        std::string chunkHeaderLine
            = _chunkedBuffer.substr(pos, chunkLineEnd - pos);
        std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
                  << "chunkHeaderLine is |" << chunkHeaderLine << "|\n";

        // Extract chunk size (if there are extensions ignore them)
        size_t semicolonPos = chunkHeaderLine.find(';');
        std::string chunkSizeStr
            = semicolonPos == std::string::npos
                  ? chunkHeaderLine
                  : chunkHeaderLine.substr(0, semicolonPos);
        size_t chunkSize = 0;
        try
        {
            chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
        }
        catch (...)
        {
            std::cout << ORANGE << "[decodeChunkedBody]" << RESET
                      << ": Invalid chunk size |" << chunkSizeStr
                      << "|, throwing exception\n";
            throw std::runtime_error("Invalid chunk size in chunked body");
        }

        std::cout << ORANGE << "[decodeChunkedBody]:" << RESET
                  << " Found the chunkHeaderLine |" << chunkHeaderLine
                  << "|, so chunkSize = " << chunkSize << " bytes\n";

        size_t chunkDataStart = chunkLineEnd + 2; // skip \r\n
        std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
                  << "Skipped \\r\\n from chunkLineEnd at pos " << chunkLineEnd
                  << ". chunkDataStart pos is " << chunkDataStart << "\n";

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
            std::string chunkData
                = _chunkedBuffer.substr(chunkDataStart, chunkSize);
            decoded += chunkData;
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
                      << "Appended chunkData |" << chunkData
                      << "|, to decoded. decoded is now |" << decoded
                      << "|, decoded.size = " << decoded.size() << "\n";

            pos = chunkDataEnd + 2; // skip chunkData + trailing \r\n
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
                      << "skipped chunkData + chunkTrailer, pos is " << pos
                      << "\n";
        }
        else
        {
            // terminating zero chunk, make sure final CRLF exists
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET << RED
                      << "reached terminating zero chunk" << RESET "\n";

            // make sure the final CRLF exists
            size_t zeroChunkEnd
                = chunkDataStart
                  + 2; // chunkDataStart points after the first \r\n
            std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
                      << "zeroChunkEnd is set to 2 bytes after chunkDataStart ("
                      << chunkDataStart << "), so at " << zeroChunkEnd << "\n";

            if (_chunkedBuffer.size() >= zeroChunkEnd
                && _chunkedBuffer[chunkDataStart] == '\r'
                && _chunkedBuffer[chunkDataStart + 1] == '\n')
            {
                pos = zeroChunkEnd;
                _terminatingZeroMet = true;
                std::cout << ORANGE << "[decodeChunkedBody]: " << RESET
                          << "Zero-size chunk fully consumed, pos = " << pos
                          << "\n";
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
              << " END: decoded is |" << decoded
              << "| with size = " << decoded.size()
              << ", bytesProcessed = " << bytesProcessed << "\n";

    return decoded;
}

bool RawRequest::conLenReached() const
{
    return _conLenBuffer.size() >= static_cast<size_t>(conLenValue());
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
    return static_cast<size_t>(conLenValue()) > _conLenBuffer.size()
               ? static_cast<size_t>(conLenValue()) - _conLenBuffer.size()
               : 0;
}

void RawRequest::consumeTempBuffer(size_t n)
{
    std::cout << MINT << "[consumeTempBuffer]: " << RESET
              << "_tempbuffer before: |" << _tempBuffer
              << "|, size = " << _tempBuffer.size() << "\n";

    if (n >= _tempBuffer.size())
    {
        _tempBuffer.clear();
    }
    else
    {
        _tempBuffer.erase(0, n); // remove the first n bytes
    }

    std::cout << MINT << "[consumeTempBuffer]: " << RESET
              << "after consuming:  |" << _tempBuffer
              << "|, size = " << _tempBuffer.size() << "\n";
}

void RawRequest::appendToBody(const std::string& data)
{
    std::cout << ORANGE << "[appendToBody]: " << RESET << "Appending "
              << data.size() << " bytes to _body\n";

    _body += data;

    std::cout << ORANGE << "[appendToBody]: " << RESET << "_body now = |"
              << _body << "|\n";
}

void RawRequest::setChunkedBuffer(std::string&& newBuffer)
{
    std::cout << ORANGE << "[setChunkedBuffer]: " << RESET
              << "old size of chunkedBuffer = " << _chunkedBuffer.size()
              << "\n";
    _chunkedBuffer = std::move(newBuffer);
    std::cout << "new size = " << _chunkedBuffer.size()
              << "(moved). content = |" << _chunkedBuffer << RESET << "|\n";
}

void RawRequest::appendBodyBytes(const std::string& data)
{
    std::cout << YELLOW << "[appendBodyBytes]:" << RESET << std::endl;

    std::cout << GREEN << "[appendBodyBytes] before appending:" << RESET << "\n"
              << "ContentLengthBuffer() = " << _conLenBuffer << "\n"
              << "ChunkedBuffer() = " << _chunkedBuffer << "\n";

    switch (_bodyType)
    {
    case BodyType::SIZED: {
        size_t remaining = remainingConLen(); // bytes still needed
        std::cout << MINT << "[appendBodyBytes]: "
                  << RESET "remaining bytes of content to append: " << remaining
                  << "\n";
        size_t toAppend = std::min(remaining, data.size());
        std::cout << MINT << "[appendBodyBytes]: "
                  << RESET "bytes to will be appended in reality: " << toAppend
                  << "\n";
        appendToConLenBuffer(data.substr(0, toAppend));
        consumeTempBuffer(toAppend); // remove exactly what we consumed
        if (conLenReached())
        {
            appendToBody(_conLenBuffer);
            setBodyDone();
        }
        std::cout << GREEN << "[appendBodyBytes] after appending:" << RESET
                  << "\n"
                  << "ContentLengthBuffer() = " << _conLenBuffer << "\n";
        std::cout
            << "[appendBodyBytes]: after finishing body length, requests:\n";
        break;
    }

    case BodyType::CHUNKED: {
        appendToChunkedBuffer(_tempBuffer);
        std::cout
            << GREEN << "[appendBodyBytes]: " << RESET
            << "appeneded _chunkBuffer with data from tempBuffer. It is now |"
            << _chunkedBuffer << "|\n";
        setTempBuffer(""); // consumed for decoding
        size_t bytesProcessed = 0;

        // decode as much as possible
        std::string decoded = decodeChunkedBody(bytesProcessed);
        appendToBody(decoded); // append only the decoded chunks

        // remove processed bytes from _chunkedBuffer
        setChunkedBuffer(_chunkedBuffer.substr(bytesProcessed));

        if (_terminatingZeroMet)
        {
            std::cout << GREEN << "[appendBodyBytes]: " << RESET
                      << "TerminatingZero found\n";
            setBodyDone();
            setTempBuffer(_chunkedBuffer);
            _chunkedBuffer.clear();
            std::cout << GREEN << "[appendBodyBytes]: " << RESET
                      << "set _tempBuffer to the contents of _chunkedBuffer, "
                         "it is now = |"
                      << _tempBuffer << "| and cleared _chunkedBuffer\n";
        }
        else
        {
            // partial chunk left? move leftovers to tempBuffer for next process
            setTempBuffer(_chunkedBuffer + _tempBuffer);

            std::cout << GREEN << "[appendBodyBytes]: " << RESET
                      << "TerminatingZero not found yet\n";
        }
        break;
    }

    case BodyType::NO_BODY:
        // Nothing to append
        break;

    case BodyType::ERROR:
        throw std::runtime_error(
            "Cannot append body data: request in ERROR state");
    }
}

void RawRequest::separateHeadersFromBody() // TODO: handle malformed requests
                                           // safely and support error pages
                                           // without URI
{
    std::cout << YELLOW << "DEBUG: separateHeadersFromBody: " << RESET
              << std::endl;
    // std::cout << "[separateHeadersFromBody] tempBuffer = |" << _tempBuffer <<
    // "|\n";

    size_t headerEnd = _tempBuffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        std::cout << "[separateHeadersFromBody] Headers incomplete "
                     "(\\r\\n\\r\\n not found)\n";
        return; // headers incomplete
    }

    std::string headerPart = _tempBuffer.substr(0, headerEnd + 4);
    // std::cout << "[separateHeadersFromBody] Header part = |" << headerPart <<
    // "|\n";

    parseRequestLineAndHeaders(headerPart);

    if (_bodyType == BodyType::NO_BODY)
    {
        std::cout << "No body, marking body done and request done\n";
        _bodyDone = true;
        _requestDone = true;
    }

    // Keep leftover (after headers) in tempBuffer
    _tempBuffer = _tempBuffer.substr(headerEnd + 4);
    std::cout << "Temp buffer after headers removed = |" << _tempBuffer
              << "|\n";

    std::cout << "Temp buffer length after header removal = "
              << _tempBuffer.size() << std::endl;
    try
    {
        _uri = normalizePath(_uri); // validate path AFTER leftovers are handled
    }
    catch (const std::exception& e)
    {
        markBadRequest(e.what());
    }
}

HttpMethod RawRequest::stringToHttpMethod(const std::string& method)
{
    if (method == "GET")
        return HttpMethod::GET;
    if (method == "POST")
        return HttpMethod::POST;
    if (method == "DELETE")
        return HttpMethod::DELETE;
    return HttpMethod::UNKNOWN; // fallback
}

void RawRequest::parseRequestLine(const std::string& firstLine)
{
    std::istringstream reqLine(firstLine);
    reqLine >> _method >> _rawUri >> _httpVersion;

    if (_method.empty() || _rawUri.empty() || _httpVersion.empty())
        throw std::runtime_error("Invalid request line");

    if (stringToHttpMethod(_method) == HttpMethod::UNKNOWN)
        throw std::invalid_argument("Unsupported HTTP method: " + _method);

    if (_httpVersion != "HTTP/1.0" && _httpVersion != "HTTP/1.1")
        throw std::invalid_argument("Unsupported HTTP version: "
                                    + _httpVersion);

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
    std::cout << "[parseRequestLine]: _uri is " << _uri << " \n";
}

std::string RawRequest::fullyDecodePercent(const std::string& rawUri)
{
    std::string decoded = rawUri;
    const int MAX_DECODE_PASSES = 4;
    int passes = 0;

    while (decoded.find('%') != std::string::npos && passes < MAX_DECODE_PASSES)
    {
        std::string once = decodePercentOnce(decoded);
        if (once == decoded)
            break;
        decoded.swap(once);
        ++passes;
    }

    if (decoded.find('%') != std::string::npos)
        throw std::runtime_error("Bad request: invalid percent-encoding or too "
                                 "many nested encodings");

    return decoded;
}

std::string RawRequest::decodePercentOnce(const std::string& s)
{
    std::string out;
    out.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '%' && i + 2 < s.size() && isHex(s[i + 1])
            && isHex(s[i + 2]))
        {
            std::string hex = s.substr(i + 1, 2);
            char decoded
                = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
            out += decoded;
            i += 2;
        }
        else
        {
            out += s[i];
        }
    }
    return out;
}

std::string RawRequest::normalizePath(const std::string& rawUri)
{
    if (rawUri.empty() || rawUri[0] != '/')
        throw std::invalid_argument("Invalid raw URI");

    // Fully decode percent-encoded sequences first
    std::string decoded = fullyDecodePercent(rawUri);

    std::vector<std::string> stack;
    std::istringstream iss(decoded);
    std::string segment;

    while (std::getline(iss, segment, '/'))
    {
        if (segment.empty() || segment == ".")
            continue;

        if (segment == "..")
        {
            if (stack.empty())
            {
                std::cout
                    << "[normalizePath] Bad request: path escapes root: \""
                    << rawUri << "\"\n";
                throw std::runtime_error("Bad request: path escapes root");
            }
            stack.pop_back();
        }
        else
        {
            stack.push_back(segment);
        }
    }

    // Rebuild normalized path
    std::ostringstream oss;
    oss << "/";
    for (size_t i = 0; i < stack.size(); ++i)
    {
        oss << stack[i];
        if (i + 1 < stack.size())
            oss << "/";
    }

    // Preserve trailing slash if original URI had it
    if (rawUri.size() > 1 && rawUri.back() == '/' && !stack.empty())
        oss << "/";

    return oss.str();
}

RequestData RawRequest::buildRequestData() const
{
    RequestData data;

    data.method = stringToHttpMethod(_method);
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

bool RawRequest::isBadRequest() const
{
    return _isBadRequest;
}

void RawRequest::printRequest(size_t idx) const
{
    std::cout << ORANGE << "[RawRequest #" << idx << "]" << RESET << "\n"
              << YELLOW << "Method: " << RESET << _method << "\n"
              << YELLOW << "URI: " << RESET << _uri << "\n"
              << YELLOW << "Query: " << RESET << _query << "\n"
              << YELLOW << "HTTP Version: " << RESET << _httpVersion << "\n"
              << YELLOW << "Body Type: " << RESET << bodyTypeToString(_bodyType)
              << "\n"
              << YELLOW << "Body Length: " << RESET << _body.size() << "\n"
              << YELLOW << "Headers Done: " << RESET
              << (_headersDone ? "true" : "false") << "\n"
              << YELLOW << "Body Done: " << RESET
              << (_bodyDone ? "true" : "false") << "\n"
              << YELLOW << "Request Done: " << RESET
              << (_requestDone ? "true" : "false") << "\n";
}

void RawRequest::printHeaders() const
{
    std::cout << ORANGE << "[Headers]" << RESET << "\n";
    for (std::unordered_map<std::string, std::string>::const_iterator it
         = _headers.begin();
         it != _headers.end(); ++it)
    {
        std::cout << GREEN << it->first << RESET << ": " << it->second << "\n";
    }
}

void RawRequest::printAllBuffers() const
{
    std::cout << ORANGE << "[Buffers]" << RESET << "\n"
              << "TempBuffer: "
              << (_tempBuffer.empty() ? "(empty)" : _tempBuffer) << "\n"
              << "RL+Headers Buffer: "
              << (_rlAndHeadersBuffer.empty() ? "(empty)" : _rlAndHeadersBuffer)
              << "\n"
              << "Chunked Buffer: "
              << (_chunkedBuffer.empty() ? "(empty)" : _chunkedBuffer) << "\n"
              << "Content-Length Buffer: "
              << (_conLenBuffer.empty() ? "(empty)" : _conLenBuffer) << "\n"
              << "Body: " << (_body.empty() ? "(empty)" : _body) << "\n";
}
