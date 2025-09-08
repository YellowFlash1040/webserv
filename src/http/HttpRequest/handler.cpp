#include "handler.hpp"

HttpResponse handleRequest(const HttpRequest& request)
{
	(void)request;
	HttpResponse response;

	response.setStatusCode(200);
	response.setBody
	(
		"<html>"
		"<body style=\"color: green; font-size: 48px; font-weight: bold; font-family: Arial, sans-serif;\">"
		"SERVER UNDER CONSTRUCTION"
		"</body>"
		"</html>"
	);
	response.addHeader("Content-Type", "text/html");

	return response;
}
