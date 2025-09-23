#include "ClientRequest.hpp"

ClientRequest::ClientRequest() = default;
ClientRequest::~ClientRequest() = default;
ClientRequest::ClientRequest(const ClientRequest& other) = default;
ClientRequest& ClientRequest::operator=(const ClientRequest& other) = default;
ClientRequest::ClientRequest(ClientRequest&& other) noexcept = default;
ClientRequest& ClientRequest::operator=(ClientRequest&& other) noexcept = default;

void ClientRequest::requestReset()
{
	_method.clear();
	_uri.clear();
	_httpVersion.clear();
	_headers.clear();
	_body.clear();
}

const std::string& ClientRequest::getMethod() const
{
	return _method;
}

const std::string& ClientRequest::getUri() const
{
	return _uri;
}

const std::string& ClientRequest::getHttpVersion() const
{
	return _httpVersion;
}

std::string ClientRequest::getHeader(const std::string& name) const
{
	auto it = _headers.find(name);
	if (it != _headers.end())
		return it->second;
	return ""; // return empty string if header not found
}

const std::map<std::string, std::string>& ClientRequest::getHeaders() const
{
	return _headers;
}

const std::string& ClientRequest::getBody() const
{
	return _body;
}

void ClientRequest::setMethod(const std::string& m)
{
	if (m != "GET" && m != "POST" && m != "DELETE")
		throw std::invalid_argument("Unsupported HTTP method: " + m);
	_method = m;
}

void ClientRequest::setUri(const std::string& u)
{
	if (u.empty() || u[0] != '/')
		throw std::invalid_argument("Invalid URI: " + u);
	_uri = u;
}

void ClientRequest::setHttpVersion(const std::string& v)
{
	if (v != "HTTP/1.0" && v != "HTTP/1.1")
		throw std::invalid_argument("Unsupported HTTP version: " + v);
	_httpVersion = v;
}

void ClientRequest::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void ClientRequest::setBody(const std::string& b)
{
	_body = b;
}
