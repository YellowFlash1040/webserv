#include "RawResponse.hpp"

RawResponse::RawResponse()
	: 
	  _statusCode(HttpStatusCode::None),
	  _statusText(""),
	  _headers(),
	  _body(),
	  _isInternalRedirect(false),
	  _mimeType(""),
	  _fileSize(0)
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

	
	void RawResponse::setMimeType(const std::string& mime)
	{
		_mimeType = mime;
	}

void RawResponse::addHeader(const std::string& key, const std::string& value)
{
	if (!hasHeader(key))
		_headers[key] = value;
}

void RawResponse::setBody(const std::string& body)
{
	_body = body;
	_headers["Content-Length"] = std::to_string(body.size());
}

void RawResponse::setDefaultHeaders()
{
	addHeader("Date", getCurrentHttpDate());
	addHeader("Server", "APT-Server/1.0");
}


void RawResponse::handleCgiScript()
{
	setStatusCode(HttpStatusCode::OK);
	addHeader("Transfer-Encoding", "chunked");
	addHeader("Content-Type", "?");
	setBody("CGI script would be executed\n");
}



bool RawResponse::shouldClose() const
{
	auto it = _headers.find("Connection");
	return it != _headers.end() && it->second == "close";
}

ResponseData RawResponse::toResponseData() const
{
    ResponseData data;

    // Basic status info
    data.statusCode = static_cast<int>(_statusCode);
    data.statusText = codeToText(_statusCode);
    data.headers = _headers;
	data.shouldClose = shouldClose();

    // Determine if we should include a body
    bool noBody = (_statusCode == HttpStatusCode::NoContent) || 
                  (_statusCode == HttpStatusCode::NotModified) || 
                  (static_cast<int>(_statusCode) >= 100 && static_cast<int>(_statusCode) < 200);

        if (!noBody)
            data.body = _body;
        else
            data.body.clear();

        if (!noBody)
            data.headers["Content-Length"] = std::to_string(data.body.size());

        data.headers["Content-Type"] = _mimeType;

        return data;

}

bool RawResponse::isInternalRedirect() const
{
	return _isInternalRedirect;
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

void RawResponse::setFileSize(size_t size)
{
	_fileSize = size;
}

size_t RawResponse::getFileSize() const
{
	return _fileSize;
}

std::string RawResponse::getErrorPageUri(const std::map<HttpStatusCode, std::string>& error_pages,
								HttpStatusCode status) const
	{
		auto it = error_pages.find(status);
		if (it != error_pages.end() && !it->second.empty())
		{
			return it->second; // e.g., "/errors/405.html"
		}
		return "";
	}

void RawResponse::addErrorDetails(const RequestContext& ctx,
											HttpStatusCode code)
{
	DBG("[addErrorDetails] Called with code: " << static_cast<int>(code));

	setStatusCode(code);
	DBG("[addErrorDetails] Code set to " << static_cast<int>(code));

	std::string pageUri = getErrorPageUri(ctx.error_pages, code);

	if (!pageUri.empty())
	{
		DBG("[addErrorDetails] Using CUSTOM error page: " << pageUri);
		setInternalRedirect(true);

		DBG("[addErrorDetails] Marked as internal redirect");
		return;
	}

	else
	{
		DBG("[addErrorDetails] Generating DEFAULT error page for "
			<< static_cast<int>(code));
		addDefaultError(code);
		addHeader("Content-Type", "text/html");
	}
}

void RawResponse::addDefaultError(HttpStatusCode code)
{
	DBG("[addDefaultError] Called for code " 
		<< static_cast<int>(code) << " (" << codeToText(code) << ")");
	
	setStatusCode(code);
		
	// _statusText = codeToText(code);

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

	DBG("[addDefaultError] Default error page generated, length = " 
		<< _body.size());
}