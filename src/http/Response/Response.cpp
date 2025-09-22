#include "Response.hpp"
#include <sstream>
#include <stdexcept>

Response::Response() = default;
Response::~Response() = default;
Response::Response(const Response& other) = default;
Response& Response::operator=(const Response& other) = default;
Response::Response(Response&& other) noexcept = default;
Response& Response::operator=(Response&& other) noexcept = default;

 void Response::responseReset()
 {
	_statusCode = 200; //??
	_statusMessage.clear();
	_headers.clear();
	_body.clear();
}

int Response::getStatusCode() const { return _statusCode; }
const std::string& Response::getStatusMessage() const { return _statusMessage; }
const std::map<std::string, std::string>& Response::getHeaders() const { return _headers; }
const std::string& Response::getBody() const { return _body; }

void Response::setStatusCode(int code)
{
	if (code < 100 || code > 599)
		throw std::invalid_argument("Invalid HTTP status code");
	_statusCode = code;
}

void Response::setStatusMessage(const std::string& message)
{
	_statusMessage = message;
}

void Response::addHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void Response::setBody(const std::string& body)
{
	_body = body;
}

std::string Response::toString() const
{
	std::ostringstream oss;
	
	// Ensure status message is set
	std::string statusMessage = _statusMessage.empty() ? "OK" : _statusMessage;
	oss << "HTTP/1.1 " << _statusCode << " " << statusMessage << "\r\n";

	bool hasContentLength = false;
	for (auto it = _headers.begin(); it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
		if (it->first == "Content-Length")
			hasContentLength = true;
	}

	// Add Content-Length if missing
	if (!hasContentLength)
		oss << "Content-Length: " << _body.size() << "\r\n";

	// Blank line before body
	oss << "\r\n";

	// Add body
	oss << _body;

	return oss.str();
}
