#include "Response.hpp"

Response::Response(const RequestData& req, const RequestContext& ctx)
	: _req(req), _ctx(ctx), _statusCode(200), _statusText("OK"), _body("")
{
	setDefaultHeaders();
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

void Response::reset()
{
	_statusCode = 200;
	_statusText = "OK";
	_headers.clear();
	_body.clear();
}

int Response::getStatusCode() const { return _statusCode; }
const std::string& Response::getStatusText() const { return _statusText; }
const std::unordered_map<std::string, std::string>& Response::getHeaders() const { return _headers; }
const std::string& Response::getBody() const { return _body; }
bool Response::hasHeader(const std::string& key) const
{
	return _headers.find(key) != _headers.end();
}

void Response::setStatusCode(int code)
{
	if (code < 100 || code > 599)
		throw std::invalid_argument("Invalid HTTP status code");
	_statusCode = code;
	_statusText = codeToText(code);
}

void Response::setStatusText(const std::string& text)
{
	_statusText = text;
}

void Response::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void Response::setBody(const std::string& body)
{
	_body = body;
	_headers["Content-Length"] = std::to_string(body.size());
}

std::string Response::toString() const
{
	std::ostringstream oss;

	oss << "HTTP/1.1 " << _statusCode << " " << _statusText << "\r\n";

	bool hasContentLength = false;
	for (auto it = _headers.begin(); it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
		if (it->first == "Content-Length")
			hasContentLength = true;
	}

	if (!hasContentLength)
		oss << "Content-Length: " << _body.size() << "\r\n";

	oss << "\r\n"; // blank line
	oss << _body;

	return oss.str();
}

std::string Response::codeToText(int code) const
{
	switch (code)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		default: return "Unknown";
	}
}

std::string Response::handleMethodNotAllowed()
{
	setStatusCode(static_cast<int>(HttpStatusCode::MethodNotAllowed));
	setStatusText("Method Not Allowed");
	addHeader("Allow", allowedMethodsToString(_ctx.allowed_methods));
	setErrorPageBody(HttpStatusCode::MethodNotAllowed, _ctx.error_pages);
	return toString();
}

void Response::setDefaultHeaders()
{
	addHeader("Date", getCurrentHttpDate());
	addHeader("Server", "APT-Server/1.0");
	addHeader("Connection", "keep-alive");
}

bool Response::isMethodAllowed(HttpMethodEnum method, const std::vector<HttpMethod>& allowed_methods)
{
	for (std::vector<HttpMethod>::const_iterator it = allowed_methods.begin(); it != allowed_methods.end(); ++it)
	{
		if (it->value() == method)
			return true;
	}
	return false;
}

std::string Response::allowedMethodsToString(const std::vector<HttpMethod>& allowed_methods)
{
	std::ostringstream oss;
	for (size_t i = 0; i < allowed_methods.size(); ++i)
	{
		oss << allowed_methods[i].toString();
		if (i + 1 < allowed_methods.size())
			oss << ", ";
	}
	return oss.str();
}

std::string Response::genResp()
{
	if (_ctx.has_return)
		return handleRedirection();
	if (!isMethodAllowed(_req.method, _ctx.allowed_methods))
		return handleMethodNotAllowed();
	if (!_ctx.cgi_pass.empty())
		return handleCgiScript();
	{	
		std::cout << "static\n";
		return handleStatic();
	}
}

std::string Response::handleRedirection()
{
	setStatusCode(static_cast<int>(HttpStatusCode::MovedPermanently));
	setStatusText("Moved Permanently");
	addHeader("Location", _ctx.redirection.url);
	setBody("Redirection in progress\n");
	return toString();
}

std::string Response::handleCgiScript()
{
	CgiRequestData cgiReq = createCgiRequest();
	// TODO: integrate with CGI handler

	setStatusCode(static_cast<int>(HttpStatusCode::Ok));
	setStatusText("OK");
	addHeader("Transfer-Encoding", "chunked");
	addHeader("Content-Type", "?");
	setBody("CGI script would be executed\n");
	return toString();
}


CgiRequestData Response::createCgiRequest()
{
	CgiRequestData cgiReq;

	// Script path is the CGI executable
	cgiReq.scriptPath = _ctx.cgi_pass;

	// HTTP method
	cgiReq.method = _req.method; // or req.method if you have a string already

	// Query string (from URL)
	cgiReq.queryString = _req.query; // implement/get it from RawRequest

	// POST body
	cgiReq.body = _req.body;

	// Fill headers
	const std::string hdrs[] =
	{
		"Content-Type",
		"Content-Length",
		"Authorization",
		"Cookie",
		"User-Agent",
		"Accept"
	};
	
	for (const std::string& h : hdrs)
	{
		std::string value = _req.getHeader(h);
		if (!value.empty())
			cgiReq.headers[h] = value;
	}
	return cgiReq;
}

std::string Response::handleStatic()
{
    FileHandler fileHandler(_ctx.root, _ctx.autoindex_enabled, _ctx.index_files);

    std::string resolvedPath;
    try
    {
        resolvedPath = fileHandler.resolveFilePath(_req.uri);
    }
    catch (const std::exception& e)
    {
        return StaticHandler::handleStaticError(fileHandler, _ctx, e);
    }

    if (fileHandler.isDirectory(resolvedPath))
    {
        if (_ctx.autoindex_enabled)
        {
            std::string dirHtml = fileHandler.handleDirectory(resolvedPath);
            setBody(dirHtml);
            addHeader("Content-Type", "text/html");
            return toString();
        }
        else
        {
			setStatusCode(404);
            setErrorPageBody(HttpStatusCode::NotFound, _ctx.error_pages);
			return toString();
		}
    }

    if (!fileHandler.fileExists(resolvedPath))
	{
		setStatusCode(404);
        setErrorPageBody(HttpStatusCode::NotFound, _ctx.error_pages);
		return toString();
	}

    std::string fileContents = fileHandler.serveFile(resolvedPath);
    setBody(fileContents);
    addHeader("Content-Type", fileHandler.detectMimeType(resolvedPath));

    return toString();
}


std::string Response::getErrorPageFilePath(
	HttpStatusCode status, const std::vector<ErrorPage>& errorPages)
{
	for (std::vector<ErrorPage>::const_iterator it = errorPages.begin();
		it != errorPages.end(); ++it)
		{
			if (std::find(it->statusCodes.begin(), it->statusCodes.end(), status) != it->statusCodes.end())
			{
				return it->filePath;
			}
		}
	return "";
}

void Response::setErrorPageBody(HttpStatusCode code, const std::vector<ErrorPage>& errorPages)
{
	std::string errorBodyPath = getErrorPageFilePath(code, errorPages);
	std::string htmlBody;

	if (!errorBodyPath.empty())
	{
		std::ifstream file(errorBodyPath);
		if (file)
		{
			std::ostringstream ss;
			ss << file.rdbuf();
			htmlBody = ss.str();
		}
	}

	// Fallback if file not found or no custom error page
	if (htmlBody.empty())
	{
		htmlBody =
			"<html>\n"
			"<head><title>" + std::to_string(static_cast<int>(code)) + " " + _statusText + "</title></head>\n"
			"<body>\n"
			"<center><h1>" + std::to_string(static_cast<int>(code)) + " " + _statusText + "</h1></center>\n"
			"<hr><center>APT-Server/1.0</center>\n"
			"</body>\n"
			"</html>\n";
	}

	setBody(htmlBody);
	addHeader("Content-Type", "text/html");
	addHeader("Content-Length", std::to_string(_body.size()));
}

bool Response::shouldClose() const
{
	auto it = _headers.find("Connection");
	return it != _headers.end() && it->second == "close";
}

