#include "parser.hpp"

static void removeCarriageReturns(std::string& str)
{
	str.erase
	(
		std::remove (str.begin(), str.end(), '\r'),
		str.end()
	);
}

 static void trimLeadingWhitespace(std::string& str)
 {
	str.erase
	(
		str.begin(),
		std::find_if(
			str.begin(),
			str.end(),
			[](unsigned char ch){ return !std::isspace(ch); }
		)
	);
}

static void parseRequestLine(std::istringstream& stream, HttpRequest& request)
{
	std::string line, method, uri, version;
	if (!std::getline(stream, line))
		throw std::runtime_error("Invalid HTTP request: empty request line");
	
	//Remove carriage return characters (\r) from a string
	removeCarriageReturns(line);
	
	std::istringstream requestStream(line);
	if (!(requestStream >> method >> uri >> version))
		throw std::runtime_error("Invalid HTTP request line");

	request.setMethod(std::move(method));
	request.setUri(std::move(uri));
	request.setHttpVersion(std::move(version));
}

static void parseHeaders(std::istringstream& stream, HttpRequest& request)
{
	std::string line;
	while (std::getline(stream, line) && line != "\r" && !line.empty())
	{
		removeCarriageReturns(line);
		auto colonPos = line.find(':');
		if (colonPos == std::string::npos)
			continue; // skip malformed header

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		
		trimLeadingWhitespace(value);
		request.addHeader(key, value);
	}
}

// In the body we don’t strip CRs (\r) because they could be meaningful content (text, HTML, JSON, or binary data)
// Only remove the very last '\n' added by the loop, keep all other newlinestel
// TODO: Currently we read until the end of the stream
// For correctness, we should parse exactly 'Content-Length' bytes (from headers)
// or handle chunked encoding in HTTP/1.1
static void parseBody(std::istringstream& stream, HttpRequest& request)
{
	std::string line, body;
	while (std::getline(stream, line))
		body += line + "\n";

	if (!body.empty() && body.back() == '\n')
		body.pop_back();

	request.setBody(std::move(body));
}

HttpRequest parseRequest(const std::string& rawRequest)
{
	HttpRequest			request;
	std::istringstream	stream(rawRequest);
	try
	{
		parseRequestLine(stream, request);
		parseHeaders(stream, request);
		parseBody(stream, request);
	} 
	catch (const std::runtime_error& e)
	{
		throw std::runtime_error(std::string("Failed to parse HTTP request: ") + e.what());
	}

	return request;
}