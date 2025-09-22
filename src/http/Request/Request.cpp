#include "Request.hpp"

Request::Request() = default;
Request::~Request() = default;
Request::Request(const Request& other) = default;
Request& Request::operator=(const Request& other) = default;
Request::Request(Request&& other) noexcept = default;
Request& Request::operator=(Request&& other) noexcept = default;

void Request::requestReset()
{
	_method.clear();
	_uri.clear();
	_httpVersion.clear();
	_headers.clear();
	_body.clear();
}

const std::string& Request::getMethod() const
{
	return _method;
}

const std::string& Request::getUri() const
{
	return _uri;
}

const std::string& Request::getHttpVersion() const
{
	return _httpVersion;
}

std::string Request::getHeader(const std::string& name) const
{
	auto it = _headers.find(name);
	if (it != _headers.end())
		return it->second;
	return ""; // return empty string if header not found
}

const std::map<std::string, std::string>& Request::getHeaders() const
{
	return _headers;
}

const std::string& Request::getBody() const
{
	return _body;
}

void Request::setMethod(const std::string& m)
{
	if (m != "GET" && m != "POST" && m != "DELETE")
		throw std::invalid_argument("Unsupported HTTP method: " + m);
	_method = m;
}

void Request::setUri(const std::string& u)
{
	if (u.empty() || u[0] != '/')
		throw std::invalid_argument("Invalid URI: " + u);
	_uri = u;
}

void Request::setHttpVersion(const std::string& v)
{
	if (v != "HTTP/1.0" && v != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + v);
	_httpVersion = v;
}

void Request::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void Request::setBody(const std::string& b)
{
	_body = b;
}
