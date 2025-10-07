#include "ParsedRequestUtils.hpp"

// Print a single ParsedRequest at index `i`
void printRequest(ClientState& clientState, size_t i)
{
	if (i >= clientState.getParsedRequestCount())
	{
		std::cout << "Index " << i << " out of range, total requests: "
				  << clientState.getParsedRequestCount() << "\n";
		return;
	}

	ParsedRequest& req = clientState.getRequest(i);
	std::cout << TEAL << "Request #" << i << RESET << "\n"
			  << "Method = " << req.getMethod()
			  << ", URI = " << req.getUri()
			  << ", BodyType = " << ParsedRequest::bodyTypeToString(req.getBodyType())
			  << ", HeadersDone = " << (req.isHeadersDone() ? "true" : "false")
			  << ", BodyDone = " << (req.isBodyDone() ? "true" : "false")
			  << ", RequestDone = " << (req.isRequestDone() ? "true" : "false")
			  << ", NeedsResponse = " << (req.needsResponse() ? "true" : "false") 
			  << "\n";
			  if (!req.getBody().empty())
				std::cout << "  Body: |" << req.getBody() << "|\n";
			if (!req.getTempBuffer().empty())
			{
				std::cout << "  tempBuffer: |" << req.getTempBuffer() << "|\n";
			}
	// Print key headers if available
	std::string host = req.getHeader("Host");
	std::string te   = req.getHeader("Transfer-Encoding");
	std::string cl   = req.getHeader("Content-Length");

	if (!host.empty())
		std::cout << "  Host: " << host << "\n";
	if (!te.empty())
		std::cout << "  Transfer-Encoding: " << te << "\n";
	if (!cl.empty())
		std::cout << "  Content-Length: " << cl << "\n";
	std::cout << std::endl;
}

// Print all requests in the ClientState
void printAllRequests(ClientState& clientState)
{
	size_t count = clientState.getParsedRequestCount();
	if (count == 0)
	{
		std::cout << "No requests in client state\n";
		return;
	}

	for (size_t i = 0; i < count; ++i)
	{
		printRequest(clientState, i);
	}
	std::cout << "\n";
}

//without last paramenter prints latest
void printSingleRequest(const ParsedRequest& req, size_t i)
{
	std::cout << TEAL << "Request #" << i << RESET << "\n"
			  << "Method = " << req.getMethod()
			  << ", URI = " << req.getUri()
			  << ", BodyType = " << req.bodyTypeToString(req.getBodyType())
			  << ", HeadersDone = " << (req.isHeadersDone() ? "true" : "false")
			  << ", BodyDone = " << (req.isBodyDone() ? "true" : "false")
			  << ", RequestDone = " << (req.isRequestDone() ? "true" : "false")
			  << ", NeedsResponse = " << (req.needsResponse() ? "true" : "false")
			  << "\n";

	if (!req.getBody().empty())
	{
		std::cout << "  Body: '" << req.getBody() << "'\n";
	}
		
	std::string host = req.getHeader("Host");
	std::string te   = req.getHeader("Transfer-Encoding");
	std::string cl   = req.getHeader("Content-Length");

	if (!host.empty()) std::cout << "  Host: " << host << "\n";
	if (!te.empty())   std::cout << "  Transfer-Encoding: " << te << "\n";
	if (!cl.empty())   std::cout << "  Content-Length: " << cl << "\n";
}

void printBodyBuffers(ParsedRequest& req)
{
	std::cout << "=== Body Buffers for Request ===\n";
	std::cout << "TempBuffer:            |" << req.getTempBuffer() << "|\n";
	std::cout << "ContentLengthBuffer:   |" << req.getContentLengthBuffer() << "|\n";
	std::cout << "ChunkedBuffer:         |" << req.getChunkedBuffer() << "|\n";
	std::cout << "Final Body (_body):    |" << req.getBody() << "|\n";
}