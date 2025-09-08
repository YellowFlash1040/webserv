#include "HttpRequest.hpp"

HttpRequest::HttpRequest() = default;
HttpRequest::~HttpRequest() = default;
HttpRequest::HttpRequest(const HttpRequest& other) = default;
HttpRequest& HttpRequest::operator=(const HttpRequest& other) = default;
HttpRequest::HttpRequest(HttpRequest&& other) noexcept = default;
HttpRequest& HttpRequest::operator=(HttpRequest&& other) noexcept = default;

const std::string& HttpRequest::getMethod() const
{
	return _method;
}

const std::string& HttpRequest::getUri() const
{
	return _uri;
}

const std::string& HttpRequest::getHttpVersion() const
{
	return _httpVersion;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const
{
	return _headers;
}

const std::string& HttpRequest::getBody() const
{
	return _body;
}

void HttpRequest::setMethod(const std::string& m)
{
	if (m != "GET" && m != "POST" && m != "DELETE")
		throw std::invalid_argument("Unsupported HTTP method: " + m);
	_method = m;
}

void HttpRequest::setUri(const std::string& u)
{
	if (u.empty() || u[0] != '/')
		throw std::invalid_argument("Invalid URI: " + u);
	_uri = u;
}

void HttpRequest::setHttpVersion(const std::string& v)
{
	if (v != "HTTP/1.0" && v != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + v);
	_httpVersion = v;
}

void HttpRequest::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void HttpRequest::setBody(const std::string& b)
{
	_body = b;
}
