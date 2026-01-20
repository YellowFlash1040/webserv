#include "RawResponse.hpp"

// -----------------------CONSTRUCTION AND DESTRUCTION-------------------------

RawResponse::RawResponse()
	: 
	  m_statusCode(HttpStatusCode::None),
	  m_statusText(""),
	  m_headers(),
	  m_body(),
	  m_isInternalRedirect(false),
	  m_mimeType(""),
	  m_fileSize(0)
{
	addDefaultHeaders();
}

// ---------------------------ACCESSORS-----------------------------

bool RawResponse::hasHeader(const std::string& key) const
{
	return m_headers.find(key) != m_headers.end();
}

bool RawResponse::isInternalRedirect() const
{
	return m_isInternalRedirect;
}

bool RawResponse::shouldClose() const
{
	auto it = m_headers.find("Connection");
	return it != m_headers.end() && it->second == "close";
}

HttpStatusCode RawResponse::statusCode() const
{
	return m_statusCode;
}

const std::string& RawResponse::statusText() const
{
	return m_statusText;
}

std::string RawResponse::header(const std::string& key) const
{
	auto it = m_headers.find(key);
	return it != m_headers.end() ? it->second : "";
}

const std::unordered_map<std::string, std::string>& RawResponse::headers() const
{
	return m_headers;
}

const std::string& RawResponse::body() const
{
	return m_body;
}

size_t RawResponse::fileSize() const
{
	return m_fileSize;
}

const std::string& RawResponse::mimeType() const
{
	return m_mimeType;
}

void RawResponse::setStatusCode(HttpStatusCode code)
{
	m_statusCode = code; 
	m_statusText = codeToText(code);
}

void RawResponse::setBody(const std::string& body)
{
	m_body = body;
	m_headers["Content-Length"] = std::to_string(body.size());
}

void RawResponse::setInternalRedirect(bool val)
{
	m_isInternalRedirect = val;
}

void RawResponse::setMimeType(const std::string& mime)
{
	m_mimeType = mime;
}

void RawResponse::setFileSize(size_t size)
{
	m_fileSize = size;
}

// ---------------------------METHODS-----------------------------

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

void RawResponse::addDefaultHeaders()
{
	addHeader("Date", getCurrentHttpDate());
	addHeader("Server", "APT-Server/1.0");
}

void RawResponse::addHeader(const std::string& key, const std::string& value)
{
	if (!hasHeader(key))
		m_headers[key] = value;
}

std::string RawResponse::lookupErrorPageUri(const std::map<HttpStatusCode, std::string>& error_pages,
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

	std::string pageUri = lookupErrorPageUri(ctx.error_pages, code);

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
	}
}

void RawResponse::addDefaultError(HttpStatusCode code)
{
	DBG("[addDefaultError] Called for code "
		<< static_cast<int>(code) << " (" << codeToText(code) << ")");
	
	setStatusCode(code);

	std::string htmlBody =
		"<html>\n"
		"<head><title>" + std::to_string(static_cast<int>(code)) + " " + m_statusText + "</title></head>\n"
		"<body>\n"
		"<center><h1>" + std::to_string(static_cast<int>(code)) + " " + m_statusText + "</h1></center>\n"
		"<center><h3>(Default Error Page)</h3></center>\n"
		"<hr><center>APT-Server/1.0</center>\n"
		"</body>\n"
		"</html>\n";

	setBody(htmlBody);
	m_mimeType = "text/html";
	addHeader("Content-Length", std::to_string(m_body.size()));

	DBG("[addDefaultError] Default error page generated, length = " 
		<< _body.size());
}

ResponseData RawResponse::toResponseData() const
{
	ResponseData data;

	// Basic status info
	data.statusCode = static_cast<int>(m_statusCode);
	data.statusText = codeToText(m_statusCode);
	data.headers = m_headers;
	data.shouldClose = shouldClose();

	// Determine if we should include a body
	bool noBody = (m_statusCode == HttpStatusCode::NoContent) || 
				  (m_statusCode == HttpStatusCode::NotModified) || 
				  (static_cast<int>(m_statusCode) >= 100 && static_cast<int>(m_statusCode) < 200);

	if (!noBody)
		data.body = m_body;
	else
		data.body.clear();

	if (!noBody)
		data.headers["Content-Length"] = std::to_string(data.body.size());

	data.headers["Content-Type"] = m_mimeType;

	return data;

}

void RawResponse::handleCgiScript()
{
	setStatusCode(HttpStatusCode::OK);
	addHeader("Transfer-Encoding", "chunked");
	addHeader("Content-Type", "?");
	setBody("CGI script would be executed\n");
}

bool RawResponse::parseFromCgiOutput(const std::string& cgiOutput)
{
	try
	{
		ParsedCGI parsed = CGIParser::parse(cgiOutput);

		setStatusCode(static_cast<HttpStatusCode>(parsed.status));
		m_headers = parsed.headers;
		m_body = parsed.body;

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
