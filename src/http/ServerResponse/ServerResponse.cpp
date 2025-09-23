#include "ServerResponse.hpp"
#include <sstream>
#include <stdexcept>

ServerResponse::ServerResponse() = default;
ServerResponse::~ServerResponse() = default;
ServerResponse::ServerResponse(const ServerResponse& other) = default;
ServerResponse& ServerResponse::operator=(const ServerResponse& other) = default;
ServerResponse::ServerResponse(ServerResponse&& other) noexcept = default;
ServerResponse& ServerResponse::operator=(ServerResponse&& other) noexcept = default;

 void ServerResponse::ServerResponseReset()
 {
	_statusCode = 200; //??
	_statusMessage.clear();
	_headers.clear();
	_body.clear();
}

int ServerResponse::getStatusCode() const { return _statusCode; }
const std::string& ServerResponse::getStatusMessage() const { return _statusMessage; }
const std::map<std::string, std::string>& ServerResponse::getHeaders() const { return _headers; }
const std::string& ServerResponse::getBody() const { return _body; }

void ServerResponse::setStatusCode(int code)
{
	if (code < 100 || code > 599)
		throw std::invalid_argument("Invalid HTTP status code");
	_statusCode = code;
}

void ServerResponse::setStatusMessage(const std::string& message)
{
	_statusMessage = message;
}

void ServerResponse::addHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void ServerResponse::setBody(const std::string& body)
{
	_body = body;
}

std::string ServerResponse::toString() const
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
