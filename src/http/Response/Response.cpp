#include "Response.hpp"

Response::Response(const RequestData& req, const RequestContext& ctx)
	: _req(req), _ctx(ctx), _body("")
{
	setStatus(HttpStatusCode::OK);
	setDefaultHeaders();
}

HttpStatusCode Response::getStatusCode() const
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

void Response::reset()
{
	_statusCode = HttpStatusCode::OK;
	_statusText = "OK";
	_headers.clear();
	_body.clear();
}


const std::string& Response::getStatusText() const { return _statusText; }
const std::unordered_map<std::string, std::string>& Response::getHeaders() const { return _headers; }
const std::string& Response::getBody() const { return _body; }
bool Response::hasHeader(const std::string& key) const
{
	return _headers.find(key) != _headers.end();
}

void Response::setStatus(HttpStatusCode code)
{
    _statusCode = code;
    _statusText = codeToText(code);
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

	oss << "HTTP/1.1 " << static_cast<int>(_statusCode) << " " << _statusText << "\r\n";


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

std::string Response::codeToText(HttpStatusCode code)
{
    switch (code)
    {
        case HttpStatusCode::OK: return "OK"; //200
        case HttpStatusCode::BadRequest: return "Bad Request"; //400
        case HttpStatusCode::NotFound: return "Not Found"; //404
        case HttpStatusCode::MethodNotAllowed: return "Method Not Allowed"; //405
        case HttpStatusCode::Forbidden: return "Forbidden"; //403
		case HttpStatusCode::PayloadTooLarge: return "Payload Too Large"; //413
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
	std::cout << "[genResp] _ctx.redirection.url.empty() = " << _ctx.redirection.url.empty()
		  << ", _ctx.redirection.statusCode = " << static_cast<int>(_ctx.redirection.statusCode)
		  << ", _ctx.redirection.url = \"" << _ctx.redirection.url << "\""
		  << ", _ctx.resolved_path = \"" << _ctx.resolved_path << "\"\n";
	
	//1. Handle external redirections
	if (!_ctx.redirection.url.empty())
		return handleRedirection();
	
	// 2. Method check (skip if internal redirect)
	// if (isInternalRedirect == false && isMethodAllowed(_req.method, _ctx.allowed_methods) == false)
	if (isMethodAllowed(_req.method, _ctx.allowed_methods) == false)
	{
		HttpStatusCode code = HttpStatusCode::MethodNotAllowed;
		setStatus(code);
		
		std::cout << "[genResp] Method allowed? " << isMethodAllowed(_req.method, _ctx.allowed_methods)
          << ", requested method = " << static_cast<int>(_req.method)
          << ", allowed_methods = " << allowedMethodsToString(_ctx.allowed_methods) << "\n";
		  
		std::cout << "[genResp] Looking for 405 page, _ctx.error_pages contains keys: ";
		for (auto &p : _ctx.error_pages)
			std::cout << static_cast<int>(p.first) << " ";
		std::cout << "\n";
		
		auto it = _ctx.error_pages.find(code);
    	if (it != _ctx.error_pages.end() && !it->second.empty())
		{
        	// Found custom error page, // Let ConnectionManager handle internal redirect
        	
			std::cout << "[genResp] found custom error page\n";
			return ""; // signals ConnectionManager to handle it
    	}
		else
		{
			setErrorPageBody(code);
			addHeader("Allow", allowedMethodsToString(_ctx.allowed_methods));
			return toString();
		}
	}
	
	//3. Payload too large
	if (_req.body.size() > _ctx.client_max_body_size)
	{
		HttpStatusCode code = HttpStatusCode::PayloadTooLarge;
		setStatus(code);
		
		auto it = _ctx.error_pages.find(code);
    	if (it != _ctx.error_pages.end() && !it->second.empty())
		{
            return "";
		}
		else
		{
			setErrorPageBody(code);
			return toString();
		}
	}
	// 4. CGI
	if (!_ctx.cgi_pass.empty())
		return handleCgiScript();

	std::cout << "[genResp] static\n";
	return handleStatic();
}

std::string Response::handleRedirection()
{
	setStatus(_ctx.redirection.statusCode);
	addHeader("Location", _ctx.redirection.url);
	setBody("Redirection in progress\n");
	return toString();
}

std::string Response::handleCgiScript()
{
	CgiRequestData cgiReq = createCgiRequest();
	// TODO: integrate with CGI handler

	setStatus(HttpStatusCode::OK);
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

    std::cout << "[handleStatic] resolvedPath = \"" << resolvedPath << "\"\n";

    if (fileHandler.isDirectory(resolvedPath))
    {
        std::cout << "[handleStatic] Path is a directory\n";

        std::string indexPath = fileHandler.getIndexFilePath(resolvedPath);
        if (!indexPath.empty())
        {
            std::cout << "[handleStatic] Found index file: " << indexPath << "\n";
            std::string fileContents = fileHandler.serveFile(indexPath);
            if (fileContents == "__FORBIDDEN__")
            {
                std::cout << "[handleStatic] Cannot read index file, returning 403\n";
                setStatus(HttpStatusCode::Forbidden);
                auto it = _ctx.error_pages.find(HttpStatusCode::Forbidden);
                if (it != _ctx.error_pages.end() && !it->second.empty())
                    return ""; // internal redirect to custom 403
                setErrorPageBody(HttpStatusCode::Forbidden);
                return toString();
            }
            setBody(fileContents);
            addHeader("Content-Type", fileHandler.detectMimeType(indexPath));
            return toString();
        }
        else
        {
            std::cout << "[handleStatic] No index file found\n";
        }

        if (_ctx.autoindex_enabled)
        {
            std::cout << "[handleStatic] Autoindex enabled, generating directory listing\n";
            std::string dirHtml = fileHandler.handleDirectory(resolvedPath);
            setBody(dirHtml);
            addHeader("Content-Type", "text/html");
            return toString();
        }
        else
        {
            std::cout << "[handleStatic] Autoindex disabled\n";
            setStatus(HttpStatusCode::NotFound);

            auto it = _ctx.error_pages.find(HttpStatusCode::NotFound);
            if (it != _ctx.error_pages.end() && !it->second.empty())
            {
                std::cout << "[handleStatic] Custom 404 page exists: " << it->second << "\n";
                std::cout << "[handleStatic] Returning empty string to trigger internal redirect\n";
                return "";
            }
            else
            {
                std::cout << "[handleStatic] No custom 404 page, using default\n";
                setErrorPageBody(HttpStatusCode::NotFound);
                return toString();
            }
        }
    }
	// File check
    std::cout << "[handleStatic] Checking if file exists: " << resolvedPath << "\n";

    if (!fileHandler.fileExists(resolvedPath))
    {
        std::cout << "[handleStatic] File does not exist: " << resolvedPath << "\n";
        setStatus(HttpStatusCode::NotFound);

        auto it = _ctx.error_pages.find(HttpStatusCode::NotFound);
        if (it != _ctx.error_pages.end() && !it->second.empty())
        {
            std::cout << "[handleStatic] Custom 404 page exists: " << it->second << "\n";
            std::cout << "[handleStatic] Returning empty string to trigger internal redirect\n";
            return "";
        }
        else
        {
            std::cout << "[handleStatic] No custom 404 page, using default\n";
            setErrorPageBody(HttpStatusCode::NotFound);
            return toString();
        }
    }

    std::cout << "[handleStatic] Serving file: " << resolvedPath << "\n";
    std::string fileContents = fileHandler.serveFile(resolvedPath);

    if (fileContents == "__FORBIDDEN__")
    {
        std::cout << "[handleStatic] Cannot read file, returning 403\n";
        setStatus(HttpStatusCode::Forbidden);
        auto it = _ctx.error_pages.find(HttpStatusCode::Forbidden);
        if (it != _ctx.error_pages.end() && !it->second.empty())
            return ""; // internal redirect to custom 403
        setErrorPageBody(HttpStatusCode::Forbidden);
        return toString();
    }

    setBody(fileContents);
    addHeader("Content-Type", fileHandler.detectMimeType(resolvedPath));
    return toString();
}

void Response::setErrorPageBody(HttpStatusCode code)
{
    // Make sure _statusText is up-to-date
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

    std::cout << "[setErrorPageBody] Default error page generated for code "
              << static_cast<int>(code) << " (" << _statusText << ")\n";
}


bool Response::shouldClose() const
{
	auto it = _headers.find("Connection");
	return it != _headers.end() && it->second == "close";
}


