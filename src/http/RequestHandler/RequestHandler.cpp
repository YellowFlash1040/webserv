#include "RequestHandler.hpp"

Response RequestHandler::handleRequest(const ClientRequest& request)
{
	(void)request; // unused for now

	Response response;
	response.setStatusCode(200);
	response.setBody(
		"<html>"
		"<body style=\"color: green; font-size: 48px; font-weight: bold; font-family: Arial, sans-serif;\">"
		"SERVER UNDER CONSTRUCTION"
		"</body>"
		"</html>"
	);
	response.addHeader("Content-Type", "text/html");

	return response;
}
