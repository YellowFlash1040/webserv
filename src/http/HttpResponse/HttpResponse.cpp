#include "HttpResponse.hpp"
#include <sstream>
#include <stdexcept>

HttpResponse::HttpResponse() = default;
HttpResponse::~HttpResponse() = default;
HttpResponse::HttpResponse(const HttpResponse& other) = default;
HttpResponse& HttpResponse::operator=(const HttpResponse& other) = default;
HttpResponse::HttpResponse(HttpResponse&& other) noexcept = default;
HttpResponse& HttpResponse::operator=(HttpResponse&& other) noexcept = default;

int HttpResponse::getStatusCode() const { return _statusCode; }
const std::string& HttpResponse::getStatusMessage() const { return _statusMessage; }
const std::map<std::string, std::string>& HttpResponse::getHeaders() const { return _headers; }
const std::string& HttpResponse::getBody() const { return _body; }

void HttpResponse::setStatusCode(int code)
{
	if (code < 100 || code > 599)
		throw std::invalid_argument("Invalid HTTP status code");
	_statusCode = code;
}

void HttpResponse::setStatusMessage(const std::string& message)
{
	_statusMessage = message;
}

void HttpResponse::addHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void HttpResponse::setBody(const std::string& body)
{
	_body = body;
}

std::string HttpResponse::toString() const
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	oss << _body;
	return oss.str();
}