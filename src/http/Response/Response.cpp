#include "Response.hpp"

Response::Response(const RequestData& req, const RequestContext& ctx)
    : _req(req), _ctx(ctx), _body("")
{
    setStatus(static_cast<int>(HttpStatusCode::Ok));
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

void Response::setStatus(int code)
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
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		default: return "Unknown";
	}
}

void Response::setDefaultHeaders()
{
	addHeader("Date", getCurrentHttpDate());
	addHeader("Server", "APT-Server/1.0");
	addHeader("Connection", "keep-alive");
}

bool Response::isMethodAllowed(HttpMethod method, const std::vector<HttpMethod>& allowed_methods)
{
    for (std::vector<HttpMethod>::const_iterator it = allowed_methods.begin(); it != allowed_methods.end(); ++it)
    {
        if (*it == method)
            return true;
    }
    return false;
}

std::string Response::httpMethodToString(HttpMethod method)
{
    switch (method)
    {
        case HttpMethod::GET:    return "GET";
        case HttpMethod::POST:   return "POST";
        case HttpMethod::PUT:    return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        default:                 return "NONE";
    }
}

std::string Response::allowedMethodsToString(const std::vector<HttpMethod>& allowed_methods)
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

std::string Response::genResp()
{
	std::cout << "[genResp] !_ctx.redirection.url.empty() = " << !_ctx.redirection.url.empty()
          << ", _ctx.redirection.statusCode = " << static_cast<int>(_ctx.redirection.statusCode)
          << ", _ctx.redirection.url = \"" << _ctx.redirection.url << "\""
          << ", _ctx.resolved_path = \"" << _ctx.resolved_path << "\"\n";
	
	if (!_ctx.redirection.url.empty())
		return handleRedirection();
	if (isMethodAllowed(_req.method, _ctx.allowed_methods) == false)
	{
    	setStatus(static_cast<int>(HttpStatusCode::MethodNotAllowed));
        setErrorPageBody(HttpStatusCode::MethodNotAllowed, _ctx.error_pages);    
		addHeader("Allow", allowedMethodsToString(_ctx.allowed_methods));
		return toString();
	}
    if (_req.body.size() > _ctx.client_max_body_size)
	{
        setStatus(static_cast<int>(HttpStatusCode::PayloadTooLarge));
        setErrorPageBody(HttpStatusCode::PayloadTooLarge, _ctx.error_pages);
		return toString();
    }
	if (!_ctx.cgi_pass.empty())
		return handleCgiScript();
	{	
		std::cout << "[genResp] static\n";
		return handleStatic();
	}
}

std::string Response::handleRedirection()
{
	setStatus(static_cast<int>(_ctx.redirection.statusCode));
	addHeader("Location", _ctx.redirection.url);
	setBody("Redirection in progress\n");
	return toString();
}

std::string Response::handleCgiScript()
{
	CgiRequestData cgiReq = createCgiRequest();
	// TODO: integrate with CGI handler

	setStatus(static_cast<int>(HttpStatusCode::Ok));
	addHeader("Transfer-Encoding", "chunked");
	addHeader("Content-Type", "?");
	setBody("CGI script would be executed\n");
	return toString();
}


CgiRequestData Response::createCgiRequest()
{
	CgiRequestData cgiReq;

    // Determine the file extension of the requested URI
    std::filesystem::path reqPath(_req.uri);
    std::string ext = reqPath.extension().string();  // includes the dot, e.g., ".php"

    // Look up the CGI executable based on extension
    auto it = _ctx.cgi_pass.find(ext);
    if (it != _ctx.cgi_pass.end())
    {
        cgiReq.scriptPath = it->second;
    }
    else
    {
        // No CGI mapping for this extension
        cgiReq.scriptPath = "";
        std::cerr << "[createCgiRequest] No CGI mapping for extension: " << ext << "\n";
    }

    // HTTP method
    cgiReq.method = _req.method;

    // Query string
    cgiReq.queryString = _req.query;

    // POST body
    cgiReq.body = _req.body;

    // Copy selected headers explicitly
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
    FileHandler fileHandler(_ctx.autoindex_enabled, _ctx.index_files);

    std::string& resolvedPath = _ctx.resolved_path;

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
			setStatus(static_cast<int>(HttpStatusCode::NotFound));
            setErrorPageBody(HttpStatusCode::NotFound, _ctx.error_pages);
			return toString();
		}
    }

    if (!fileHandler.fileExists(resolvedPath))
	{
		setStatus(static_cast<int>(HttpStatusCode::NotFound));
        setErrorPageBody(HttpStatusCode::NotFound, _ctx.error_pages);
		return toString();
	}

    std::string fileContents = fileHandler.serveFile(resolvedPath);
    setBody(fileContents);
    addHeader("Content-Type", fileHandler.detectMimeType(resolvedPath));

    return toString();
}

void Response::setErrorPageBody(HttpStatusCode code, const std::map<HttpStatusCode, std::string>& errorPages)
{
    std::string htmlBody;
    auto it = errorPages.find(code);
    if (it != errorPages.end())
    {
        std::ifstream file(it->second);
        if (file)
        {
            std::ostringstream ss;
            ss << file.rdbuf();
            htmlBody = ss.str();
        }
    }
	else
	{
		 std::cout << "[setErrorPageBody] could not open error page file: " << it->second
              << " (exists=" << std::filesystem::exists(it->second)
              << ", perm ok=" << ((std::filesystem::exists(it->second) &&
                  (std::filesystem::status(it->second).permissions() &
                   std::filesystem::perms::owner_read) != std::filesystem::perms::none) ? "yes" : "no")
              << ")\n";
	}

    // fallback if file missing
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
