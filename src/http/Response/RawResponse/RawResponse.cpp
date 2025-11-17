#include "RawResponse.hpp"

RawResponse::RawResponse(const RequestData& req, const RequestContext& ctx)
    : _req(req),
      _ctx(ctx),
      _statusCode(HttpStatusCode::OK),
      _statusText(""),
      _headers(),
      _body(""),
      _isInternalRedirect(false),
      _isExternalRedirect(false),
      _redirectTarget(""),
      _fileMode(FileDeliveryMode::InMemory),
      _filePath(""),
      _mimeType("")
{
    setDefaultHeaders();
}

HttpStatusCode RawResponse::getStatusCode() const
{
	return _statusCode;
}

std::string getCurrentHttpDate()
{

	// Get current time as system_clock time_point
	auto now = std::chrono::system_clock::now();

	// Convert to time_t (seconds since epoch)
	std::time_t t = std::chrono::system_clock::to_time_t(now);

	// Convert to GMT/UTC time
	std::tm gmt = *std::gmtime(&t);

	// Format according to RFC 7231: "Day, DD Mon YYYY HH:MM:SS GMT"
	std::ostringstream oss;
	oss << std::put_time(&gmt, "%a, %d %b %Y %H:%M:%S GMT");
	return oss.str();
}

const std::string& RawResponse::getStatusText() const { return _statusText; }
const std::unordered_map<std::string, std::string>& RawResponse::getHeaders() const { return _headers; }

bool RawResponse::hasHeader(const std::string& key) const
{
	return _headers.find(key) != _headers.end();
}

void RawResponse::setStatus(HttpStatusCode code)
{
	_statusCode = code;
	_statusText = codeToText(code);
}

void RawResponse::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void RawResponse::setBody(const std::string& body)
{
	_body = body;
	_headers["Content-Length"] = std::to_string(body.size());
}

std::string RawResponse::codeToText(HttpStatusCode code)
{
	switch (code)
	{
		case HttpStatusCode::OK: return "OK"; //200
		case HttpStatusCode::NoContent: return "No Content"; //204
		case HttpStatusCode::BadRequest: return "Bad Request"; //400
		case HttpStatusCode::Forbidden: return "Forbidden"; //403
		case HttpStatusCode::NotFound: return "Not Found"; //404
		case HttpStatusCode::MethodNotAllowed: return "Method Not Allowed"; //405
		case HttpStatusCode::PayloadTooLarge: return "Payload Too Large"; //413
		case HttpStatusCode::InternalServerError: return "Internal Server Error"; //500
		case HttpStatusCode::NotImplemented: return "Not Implemented"; //501
		case HttpStatusCode::BadGateway: return "Bad Gateway"; //502
		case HttpStatusCode::LoopDetected: return "Loop Detected"; //508
		
		default: return "Unknown";
	}
}

void RawResponse::setDefaultHeaders()
{
	addHeader("Date", getCurrentHttpDate());
	addHeader("Server", "APT-Server/1.0");
	addHeader("Connection", "keep-alive");
}



std::string RawResponse::httpMethodToString(HttpMethod method)
{
	switch (method)
	{
		case HttpMethod::GET:    return "GET";
		case HttpMethod::POST:   return "POST";
		case HttpMethod::DELETE: return "DELETE";
		default:                 return "NONE";
	}
}

std::string RawResponse::allowedMethodsToString(const std::vector<HttpMethod>& allowed_methods)
{
	std::ostringstream oss;
	for (size_t i = 0; i < allowed_methods.size(); ++i)
	{
		oss << httpMethodToString(allowed_methods[i]);
		if (i < allowed_methods.size() - 1)
			oss << ", ";
	}
	return oss.str();
}

void RawResponse::sendFile(const std::string &filePath, const std::string &mimeType)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "[sendFile] ERROR: cannot open file: " << filePath << std::endl;
		setStatus(HttpStatusCode::NotFound);
		setBody("<html><body><h1>404 Not Found</h1></body></html>");
		addHeader("Content-Type", "text/html");
		return;
	}

	// Determine file size
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(static_cast<size_t>(size));
	if (!file.read(buffer.data(), size)) {
		std::cout << "[sendFile] ERROR: failed to read file: " << filePath << std::endl;
		setStatus(HttpStatusCode::InternalServerError);
		setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
		addHeader("Content-Type", "text/html");
		return;
	}

	file.close();

	// Set response headers
	setStatus(HttpStatusCode::OK);
	addHeader("Content-Type", mimeType);
	addHeader("Content-Length", std::to_string(size));

	// Set the body from the file content
	setBody(std::string(buffer.begin(), buffer.end()));

	std::cout << "[sendFile] Served file: " << filePath << ", size=" << size << std::endl;
}

void RawResponse::handleExternalRedirect(const std::string& reqUri)
{
	if (_ctx.redirection.url == reqUri)
	{
		// Handle self-redirect differently
		setStatus(HttpStatusCode::LoopDetected);
		setBody("<html><head><title>Error</title></head>"
				"<body>Redirection loop detected for " + _ctx.redirection.url + "</body></html>");
	}
	else
	{
		// Standard external redirection
		_isExternalRedirect = true;
		_redirectTarget = _ctx.redirection.url;
		setStatus(_ctx.redirection.statusCode);
		setBody("<html><head><title>Moved</title></head>"
				"<body>Redirection in progress. <a href=\"" + _ctx.redirection.url + "\">Click here</a></body></html>");

	}
	addHeader("Location", _ctx.redirection.url);
	addHeader("Content-Length", std::to_string(_body.size()));
	addHeader("Content-Type", "text/html");
}

void RawResponse::handleCgiScript()
{
	setStatus(HttpStatusCode::OK);
	addHeader("Transfer-Encoding", "chunked");
	addHeader("Content-Type", "?");
	setBody("CGI script would be executed\n");
}

void RawResponse::addDefaultErrorDetails(HttpStatusCode code)
{
	_statusText = codeToText(code);

	std::string htmlBody =
		"<html>\n"
		"<head><title>" + std::to_string(static_cast<int>(code)) + " " + _statusText + "</title></head>\n"
		"<body>\n"
		"<center><h1>" + std::to_string(static_cast<int>(code)) + " " + _statusText + "</h1></center>\n"
		"<center><h3>(Default Error Page)</h3></center>\n"
		"<hr><center>APT-Server/1.0</center>\n"
		"</body>\n"
		"</html>\n";

	setBody(htmlBody);
	addHeader("Content-Type", "text/html");
	addHeader("Content-Length", std::to_string(_body.size()));

	std::cout << "[addDefaultErrorDetails] Default error page generated for code "
			  << static_cast<int>(code) << " (" << _statusText << ")\n";
}

bool RawResponse::shouldClose() const
{
	auto it = _headers.find("Connection");
	return it != _headers.end() && it->second == "close";
}

ResponseData RawResponse::toResponseData() const
{
	ResponseData rd;
	rd.statusCode = static_cast<int>(_statusCode);
	rd.statusText = _statusText;
	rd.body = _body;
	rd.headers = _headers;
	rd.shouldClose = shouldClose();
	return rd;
}

bool RawResponse::isInternalRedirect() const
{
	return _isInternalRedirect;
}

bool RawResponse::isExternalRedirect() const
{
	return _isExternalRedirect;
}

void RawResponse::setInternalRedirect(bool val)
{
	_isInternalRedirect = val;
}

std::string RawResponse::getHeader(const std::string& key) const
{
	auto it = _headers.find(key);
	return it != _headers.end() ? it->second : "";
}

void RawResponse::setFileContent(const std::string& content, const std::string& mimeType)
{
	_body = content;
	_mimeType = mimeType;
	_fileMode = FileDeliveryMode::InMemory;
	_headers["Content-Type"] = mimeType;
	_headers["Content-Length"] = std::to_string(content.size());
}

void RawResponse::setFilePath(const std::string& path, const std::string& mimeType)
{
	_filePath = path;
	_mimeType = mimeType;
	_fileMode = FileDeliveryMode::Streamed;
	_headers["Content-Type"] = mimeType;
}

FileDeliveryMode RawResponse::getFileMode() const
{
	return _fileMode;
}

const std::string& RawResponse::getFilePath() const
{
	return _filePath;
}

const std::string& RawResponse::getBody() const
{
	return _body;
}

const std::string& RawResponse::getMimeType() const
{
	return _mimeType;
}

void RawResponse::setStatusCode(HttpStatusCode code)
{
	_statusCode = code; _statusText = codeToText(code);
}

void RawResponse::setRedirectTarget(const std::string& uri) 
{
	_redirectTarget = uri;
}

const std::string& RawResponse::getRedirectTarget() const
{
	return _redirectTarget;
}
