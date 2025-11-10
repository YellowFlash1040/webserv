#include "RawResponse.hpp"

RawResponse::RawResponse(const RequestData& req, const RequestContext& ctx)
  : _req(req)
  , _ctx(ctx)
  , _body("")
  , _isInternalRedirect(false)
{
    setStatus(HttpStatusCode::Ok);
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

// void RawResponse::reset()
// {
// 	_statusCode = HttpStatusCode::OK;
// 	_statusText = "OK";
// 	_headers.clear();
// 	_body.clear();
// }

const std::string& RawResponse::getStatusText() const
{
    return _statusText;
}
const std::unordered_map<std::string, std::string>& RawResponse::getHeaders()
    const
{
    return _headers;
}
const std::string& RawResponse::getBody() const
{
    return _body;
}
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

// std::string RawResponse::toString() const
// {
// 	std::ostringstream oss;

// 	oss << "HTTP/1.1 " << static_cast<int>(_statusCode) << " " << _statusText <<
// "\r\n";

// 	bool hasContentLength = false;
// 	for (auto it = _headers.begin(); it != _headers.end(); ++it)
// 	{
// 		oss << it->first << ": " << it->second << "\r\n";
// 		if (it->first == "Content-Length")
// 			hasContentLength = true;
// 	}

// 	if (!hasContentLength)
// 		oss << "Content-Length: " << _body.size() << "\r\n";

// 	oss << "\r\n"; // blank line
// 	oss << _body;

// 	return oss.str();
// }

std::string RawResponse::codeToText(HttpStatusCode code)
{
    switch (code)
    {
    case HttpStatusCode::Ok:
        return "OK"; // 200
    case HttpStatusCode::BadRequest:
        return "Bad Request"; // 400
    case HttpStatusCode::NotFound:
        return "Not Found"; // 404
    case HttpStatusCode::MethodNotAllowed:
        return "Method Not Allowed"; // 405
    case HttpStatusCode::Forbidden:
        return "Forbidden"; // 403
    case HttpStatusCode::PayloadTooLarge:
        return "Payload Too Large"; // 413
    default:
        return "Unknown";
    }
}

void RawResponse::setDefaultHeaders()
{
    addHeader("Date", getCurrentHttpDate());
    addHeader("Server", "APT-Server/1.0");
    addHeader("Connection", "keep-alive");
}

bool RawResponse::isMethodAllowed(
    HttpMethod method, const std::vector<HttpMethod>& allowed_methods)
{
    for (std::vector<HttpMethod>::const_iterator it = allowed_methods.begin();
         it != allowed_methods.end(); ++it)
    {
        if (*it == method)
            return true;
    }
    return false;
}

std::string RawResponse::httpMethodToString(HttpMethod method)
{
    switch (method)
    {
    case HttpMethod::GET:
        return "GET";
    case HttpMethod::POST:
        return "POST";
    case HttpMethod::DELETE:
        return "DELETE";
    default:
        return "NONE";
    }
}

std::string RawResponse::allowedMethodsToString(
    const std::vector<HttpMethod>& allowed_methods)
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

void RawResponse::genResp()
{
    std::cout << "[genResp] _ctx.redirection.url.empty() = "
              << _ctx.redirection.url.empty()
              << ", _ctx.redirection.statusCode = "
              << static_cast<int>(_ctx.redirection.statusCode)
              << ", _ctx.redirection.url = \"" << _ctx.redirection.url << "\""
              << ", _ctx.resolved_path = \"" << _ctx.resolved_path << "\"\n";

    // 1. External redirection - TO DO
    if (!_ctx.redirection.url.empty())
    {
        handleRedirection();
        return;
    }

    // 2. Method not allowed
    if (!isMethodAllowed(_req.method, _ctx.allowed_methods))
    {
        setStatus(HttpStatusCode::MethodNotAllowed);
        auto it = _ctx.error_pages.find(HttpStatusCode::MethodNotAllowed);
        if (it != _ctx.error_pages.end() && !it->second.empty())
        {
            _isInternalRedirect = true; // signal to ConnectionManager
            return;
        }
        else
        {
            generateDefaultErrorPage(HttpStatusCode::MethodNotAllowed);
            addHeader("Allow", allowedMethodsToString(_ctx.allowed_methods));
            return;
        }
    }

    // 3. Payload too large
    if (_req.body.size() > _ctx.client_max_body_size)
    {
        setStatus(HttpStatusCode::PayloadTooLarge);
        auto it = _ctx.error_pages.find(HttpStatusCode::PayloadTooLarge);
        if (it != _ctx.error_pages.end() && !it->second.empty())
        {
            _isInternalRedirect = true;
            return;
        }
        else
        {
            generateDefaultErrorPage(HttpStatusCode::PayloadTooLarge);
            return;
        }
    }

    // 4. CGI
    if (!_ctx.cgi_pass.empty())
    {
        handleCgiScript();
        return;
    }

    // 5. Static
    handleStatic();
}

void RawResponse::handleRedirection()
{
    setStatus(_ctx.redirection.statusCode);
    addHeader("Location", _ctx.redirection.url);
    setBody("Redirection in progress\n");
}

void RawResponse::handleCgiScript()
{
    setStatus(HttpStatusCode::Ok);
    addHeader("Transfer-Encoding", "chunked");
    addHeader("Content-Type", "?");
    setBody("CGI script would be executed\n");
}

void RawResponse::handleStatic()
{
    FileHandler fileHandler(_ctx.autoindex_enabled, _ctx.index_files);
    std::string& resolvedPath = _ctx.resolved_path;

    std::cout << "[handleStatic] resolvedPath = \"" << resolvedPath << "\"\n";

    bool isDir = fileHandler.isDirectory(resolvedPath);

    // 1. Handle directories first
    if (isDir)
    {
        std::cout << "[handleStatic] Path is a directory\n";

        // Try to serve an index file
        std::string indexPath = fileHandler.getIndexFilePath(resolvedPath);
        if (!indexPath.empty())
        {
            std::cout << "[handleStatic] Found index file: " << indexPath
                      << "\n";
            std::string fileContents = fileHandler.serveFile(indexPath);
            if (fileContents == "__FORBIDDEN__")
            {
                std::cout
                    << "[handleStatic] Cannot read index file, returning 403\n";
                setStatus(HttpStatusCode::Forbidden);
                auto it = _ctx.error_pages.find(HttpStatusCode::Forbidden);
                if (it != _ctx.error_pages.end() && !it->second.empty())
                {
                    _isInternalRedirect
                        = true; // internal redirect to custom 403
                    return;
                }
                generateDefaultErrorPage(HttpStatusCode::Forbidden);
                return;
            }
            setBody(fileContents);
            addHeader("Content-Type", fileHandler.detectMimeType(indexPath));
            return; // stop here if index file served
        }

        // No index file, check autoindex
        if (_ctx.autoindex_enabled)
        {
            std::cout << "[handleStatic] Autoindex enabled, generating "
                         "directory listing\n";
            std::string dirHtml = fileHandler.handleDirectory(resolvedPath);
            setBody(dirHtml);
            addHeader("Content-Type", "text/html");
            return; // stop here if directory listing served
        }

        // Directory without index file and autoindex disabled → 404
        std::cout << "[handleStatic] Autoindex disabled, directory without "
                     "index file\n";
        setStatus(HttpStatusCode::NotFound);
        auto it = _ctx.error_pages.find(HttpStatusCode::NotFound);
        if (it != _ctx.error_pages.end() && !it->second.empty())
        {
            std::cout << "[handleStatic] Custom 404 page exists: " << it->second
                      << "\n";
            _isInternalRedirect = true;
            return;
        }
        generateDefaultErrorPage(HttpStatusCode::NotFound);
        return;
    }

    // 2. Handle regular files
    std::cout << "[handleStatic] Checking if file exists: " << resolvedPath
              << "\n";
    if (!fileHandler.fileExists(resolvedPath))
    {
        std::cout << "[handleStatic] File does not exist: " << resolvedPath
                  << "\n";
        setStatus(HttpStatusCode::NotFound);

        auto it = _ctx.error_pages.find(HttpStatusCode::NotFound);
        if (it != _ctx.error_pages.end() && !it->second.empty())
        {
            std::cout << "[handleStatic] Custom 404 page exists: " << it->second
                      << "\n";
            _isInternalRedirect = true;
            return;
        }
        generateDefaultErrorPage(HttpStatusCode::NotFound);
        return;
    }

    // 3. Serve regular file
    std::cout << "[handleStatic] Serving file: " << resolvedPath << "\n";
    std::string fileContents = fileHandler.serveFile(resolvedPath);
    if (fileContents == "__FORBIDDEN__")
    {
        std::cout << "[handleStatic] Cannot read file, returning 403\n";
        setStatus(HttpStatusCode::Forbidden);
        auto it = _ctx.error_pages.find(HttpStatusCode::Forbidden);
        if (it != _ctx.error_pages.end() && !it->second.empty())
        {
            _isInternalRedirect = true; // internal redirect to custom 403
            return;
        }
        generateDefaultErrorPage(HttpStatusCode::Forbidden);
        return;
    }

    setBody(fileContents);
    addHeader("Content-Type", fileHandler.detectMimeType(resolvedPath));
}

void RawResponse::generateDefaultErrorPage(HttpStatusCode code)
{
    _statusText = codeToText(code);

    std::string htmlBody
        = "<html>\n"
          "<head><title>"
          + std::to_string(static_cast<int>(code)) + " " + _statusText
          + "</title></head>\n"
            "<body>\n"
            "<center><h1>"
          + std::to_string(static_cast<int>(code)) + " " + _statusText
          + "</h1></center>\n"
            "<center><h3>(Default Error Page)</h3></center>\n"
            "<hr><center>APT-Server/1.0</center>\n"
            "</body>\n"
            "</html>\n";

    setBody(htmlBody);
    addHeader("Content-Type", "text/html");
    addHeader("Content-Length", std::to_string(_body.size()));

    std::cout
        << "[generateDefaultErrorPage] Default error page generated for code "
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
