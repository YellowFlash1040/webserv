#include "RequestParser.hpp"

// Helpers
void RequestParser::removeCarriageReturns(std::string& str)
{
	str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
}

void RequestParser::trimLeadingWhitespace(std::string& str)
{
	str.erase(
		str.begin(),
		std::find_if(str.begin(), str.end(),
					 [](unsigned char ch) { return !std::isspace(ch); })
	);
}

// Parsing functions
bool RequestParser::headersParsed(const ClientState& state) const
{
	return state.getHeaderBuffer().find("\r\n\r\n") != std::string::npos;
}

// Extract Content-Length from headers
size_t RequestParser::extractContentLength(const ClientState& state) const
{
	const std::string headerName = "Content-Length:";
	auto pos = state.getHeaderBuffer().find(headerName);
	if (pos == std::string::npos)
		return 0;

	pos += headerName.size();
	std::string rest = state.getHeaderBuffer().substr(pos);
	trimLeadingWhitespace(rest);

	size_t end = rest.find("\r\n");
	if (end != std::string::npos)
		rest = rest.substr(0, end);

	return static_cast<size_t>(std::stoul(rest));
}

// Check if body has been completely received
bool RequestParser::bodyComplete(const ClientState& state) const
{
	if (state.isChunked())
		return false; // TODO: implement chunked
	return state.getBodyBuffer().size() >= state.getContentLength();

}

// Parse a complete request from raw string
ClientRequest RequestParser::parseCompleteRequest(ClientState& state) const
{
	ClientRequest request;

	auto pos = state.getHeaderBuffer().find("\r\n\r\n");
	if (pos == std::string::npos)
		throw std::runtime_error("Headers not complete");

	std::string headerPart = state.getHeaderBuffer().substr(0, pos + 2);
	std::string bodyPart = state.getBodyBuffer();

	std::istringstream stream(headerPart);
	std::string line;

	// Request line
	if (!std::getline(stream, line))
		throw std::runtime_error("Empty request line");
	removeCarriageReturns(line);
	std::istringstream reqLine(line);
	std::string method, uri, version;
	reqLine >> method >> uri >> version;
	request.setMethod(method);
	request.setUri(uri);
	request.setHttpVersion(version);

	// Headers
	while (std::getline(stream, line) && !line.empty() && line != "\r")
	{
		removeCarriageReturns(line);
		auto colonPos = line.find(':');
		if (colonPos == std::string::npos)
			continue;
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		trimLeadingWhitespace(value);
		request.addHeader(key, value);
	}

	request.setBody(bodyPart);

	// Update ClientState
	state.setHeadersComplete(true);
	state.setContentLength(extractContentLength(state));

	return request;
}
