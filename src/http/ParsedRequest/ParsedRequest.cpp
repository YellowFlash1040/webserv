#include "ParsedRequest.hpp"

ParsedRequest::ParsedRequest() = default;
ParsedRequest::~ParsedRequest() = default;
ParsedRequest::ParsedRequest(const ParsedRequest& other) = default;
ParsedRequest& ParsedRequest::operator=(const ParsedRequest& other) = default;
ParsedRequest::ParsedRequest(ParsedRequest&& other) noexcept = default;
ParsedRequest& ParsedRequest::operator=(ParsedRequest&& other) noexcept = default;

void ParsedRequest::requestReset()
{
	_method.clear();
	_uri.clear();
	_httpVersion.clear();
	_headers.clear();
	_body.clear();
}

const std::string& ParsedRequest::getMethod() const
{
	return _method;
}

const std::string& ParsedRequest::getUri() const
{
	return _uri;
}

const std::string& ParsedRequest::getHttpVersion() const
{
	return _httpVersion;
}

std::string ParsedRequest::getHeader(const std::string& name) const
{
	auto it = _headers.find(name);
	if (it != _headers.end())
		return it->second;
	return ""; // return empty string if header not found
}

const std::map<std::string, std::string>& ParsedRequest::getHeaders() const
{
	return _headers;
}

const std::string& ParsedRequest::getBody() const
{
	return _body;
}

void ParsedRequest::setMethod(const std::string& m)
{
	if (m != "GET" && m != "POST" && m != "DELETE")
		throw std::invalid_argument("Unsupported HTTP method: " + m);
	_method = m;
}

void ParsedRequest::setUri(const std::string& u)
{
	if (u.empty() || u[0] != '/')
		throw std::invalid_argument("Invalid URI: " + u);
	_uri = u;
}

void ParsedRequest::setHttpVersion(const std::string& v)
{
	if (v != "HTTP/1.0" && v != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + v);
	_httpVersion = v;
}

void ParsedRequest::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void ParsedRequest::setBody(const std::string& b)
{
	_body = b;
}
