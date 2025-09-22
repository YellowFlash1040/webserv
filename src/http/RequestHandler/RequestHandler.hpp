#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

class RequestHandler
{
public:
	// Canonical form
	RequestHandler() = default;
	~RequestHandler() = default;

	RequestHandler(const RequestHandler& other) = default;
	RequestHandler& operator=(const RequestHandler& other) = default;

	RequestHandler(RequestHandler&& other) noexcept = default;
	RequestHandler& operator=(RequestHandler&& other) noexcept = default;

	// Handle a request
	Response handleRequest(const Request& request);
};

#endif
