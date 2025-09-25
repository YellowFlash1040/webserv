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
	return state.getRlAndHeadersBuffer().find("\r\n\r\n") != std::string::npos;
}

// Extract Content-Length from headers
size_t RequestParser::extractContentLength(const ClientState& state) const
{
	const std::string headerName = "Content-Length:";
	auto pos = state.getRlAndHeadersBuffer().find(headerName);
	if (pos == std::string::npos)
		return 0;

	pos += headerName.size();
	std::string rest = state.getRlAndHeadersBuffer().substr(pos);
	trimLeadingWhitespace(rest);

	size_t end = rest.find("\r\n");
	if (end != std::string::npos)
		rest = rest.substr(0, end);

	return static_cast<size_t>(std::stoul(rest));
}

// Check if body has been completely received
bool RequestParser::isBodyDone(const ClientState& state) const
{
	if (state.isChunked())
	{
		// Look for the "0\r\n\r\n" terminator
		return state.getBodyBuffer().find("\r\n0\r\n\r\n") != std::string::npos;
	}
	else if (state.getContentLength() > 0)
	{
		return state.getBodyBuffer().size() >= state.getContentLength();
	}
	else
	{
		// No body expected (e.g. GET)
		return true;
	}
}

// Parse a complete request from raw string
ParsedRequest RequestParser::parseBufferedRequest(ClientState& clientState) const
{
	ParsedRequest request;

	auto pos = clientState.getRlAndHeadersBuffer().find("\r\n\r\n");
	if (pos == std::string::npos)
		throw std::runtime_error("Headers not complete");

	std::string headerPart = clientState.getRlAndHeadersBuffer().substr(0, pos + 2);
	std::string bodyPart = clientState.getBodyBuffer();

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
	clientState.setHeadersDone();
	clientState.setContentLength(extractContentLength(clientState));

	return request;
}
