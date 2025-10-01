#include "RequestHandler.hpp"

ServerResponse RequestHandler::handleRequest(const ParsedRequest& request)
{
	(void)request; // unused for now

	ServerResponse response;
	response.setStatusCode(200);
	response.setBody(
		"<html>"
		"<body style=\"color: green; font-size: 48px; font-weight: bold; font-family: Arial, sans-serif;\">"
		"SERVER UNDER CONSTRUCTION"
		"</body>"
		"</html>"
	);
	// response.setStatusCode(200);
    // response.setStatusText("OK");
    // response.setHeader("Content-Type", "text/html");
    // response.setBody("<html><body><h1>Hello</h1></body></html>");
	// response.addHeader("Content-Type", "text/html");

	return response;
}
