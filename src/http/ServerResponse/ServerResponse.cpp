#include "ServerResponse.hpp"
#include <sstream>     // for ostringstream
#include <stdexcept>   // for invalid_argument


ServerResponse::ServerResponse() 
  : _statusCode(200), _statusText("OK"), _headers({{"Content-Type","text/html"},{"Connection","keep-alive"}}), _body("")
{}

ServerResponse::ServerResponse(const ParsedRequest& req)
    : _statusCode(200),
      _statusText("OK"),
      _headers({{"Content-Type", "text/html"}, {"Connection", "keep-alive"}}),
      _body("<html><body>Requested URI: " + req.getUri() + "</body></html>")
{}

void ServerResponse::reset()
{
	_statusCode = 200;
	_statusText = "OK";
	_headers.clear();
	_body.clear();
}

int ServerResponse::getStatusCode() const { return _statusCode; }
const std::string& ServerResponse::getStatusText() const { return _statusText; }
const std::unordered_map<std::string, std::string>& ServerResponse::getHeaders() const { return _headers; }
const std::string& ServerResponse::getBody() const { return _body; }
bool ServerResponse::hasHeader(const std::string& key) const
{
	return _headers.find(key) != _headers.end();
}

void ServerResponse::setStatusCode(int code)
{
	if (code < 100 || code > 599)
		throw std::invalid_argument("Invalid HTTP status code");
	_statusCode = code;
	_statusText = codeToText(code);
}

void ServerResponse::setStatusText(const std::string& text) { _statusText = text; }
void ServerResponse::addHeader(const std::string& key, const std::string& value) { _headers[key] = value; }
void ServerResponse::setBody(const std::string& body)
{
	_body = body;
	_headers["Content-Length"] = std::to_string(body.size());
}

std::string ServerResponse::toString() const
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

std::string ServerResponse::codeToText(int code) const
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
