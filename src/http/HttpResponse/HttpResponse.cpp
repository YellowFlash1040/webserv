#include "HttpResponse.hpp"
#include <sstream>
#include <stdexcept>

HttpResponse::HttpResponse() = default;
HttpResponse::~HttpResponse() = default;
HttpResponse::HttpResponse(const HttpResponse& other) = default;
HttpResponse& HttpResponse::operator=(const HttpResponse& other) = default;
HttpResponse::HttpResponse(HttpResponse&& other) noexcept = default;
HttpResponse& HttpResponse::operator=(HttpResponse&& other) noexcept = default;

int HttpResponse::getStatusCode() const
{
	return _statusCode;
}

const std::string& HttpResponse::getStatusMessage() const
{ 
	return _statusMessage;
}

const std::map<std::string, std::string>& HttpResponse::getHeaders() const
{ 
	return _headers;
}

const std::string& HttpResponse::getBody() const
{
	return _body;
}

bool HttpResponse::hasHeader(const std::string& key) const
{
		return _headers.find(key) != _headers.end();
}

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
	std::ostringstream outStream;

	// Status line
	outStream << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";
	
	// Ensure Content-Length header is present (if body exists and not chunked)
	if (!hasHeader("Content-Length") && !hasHeader("Transfer-Encoding"))
		outStream << "Content-Length: " << _body.size() << "\r\n";
	
	// Default headers (e.g., Content-Type)
	for (const auto& header : _headers)
	{
		outStream << header.first << ": " << header.second << "\r\n";
	}
	
	// End of headers
	outStream << "\r\n";

	//Body
	outStream << _body;
	
	return outStream.str();
}