#include "Response.hpp"
#include <sstream>     // for ostringstream
#include <stdexcept>   // for invalid_argument


Response::Response() 
  : _statusCode(200), _statusText("OK"), _headers({{"Content-Type","text/html"},{"Connection","keep-alive"}}), _body("")
{}

Response::Response(const ParsedRequest& req)
	: _statusCode(200),
	  _statusText("OK"),
	  _headers({{"Content-Type", "text/html"}, {"Connection", "keep-alive"}}),
	  _body("<html><body>Requested URI: " + req.getUri() + "</body></html>")
{}

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

void Response::setStatusText(const std::string& text) { _statusText = text; }
void Response::addHeader(const std::string& key, const std::string& value) { _headers[key] = value; }
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
	
std::string Response::genResp(const ParsedRequest& req)
{
    // Create a Response object from the request
    
	if (!req.isRequestDone())
		return ""; // nothing ready
	
	Response resp(req);

    // Hardcode response for testing
    resp.setStatusCode(200);
    resp.setStatusText("OK");
    resp.addHeader("Content-Length", "13");
    resp.addHeader("Content-Type", "text/plain");
    resp.setBody("Hello, world!");

    // Serialize the response to a string
    return resp.toString();
}