#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../ParsedRequest/ParsedRequest.hpp"
#include "../ServerResponse/ServerResponse.hpp"

class RequestHandler
{
	public:
		RequestHandler() = default;
		~RequestHandler() = default;
		RequestHandler(const RequestHandler& other) = default;
		RequestHandler& operator=(const RequestHandler& other) = default;
		RequestHandler(RequestHandler&& other) noexcept = default;
		RequestHandler& operator=(RequestHandler&& other) noexcept = default;

		ServerResponse handleRequest(const ParsedRequest& request);
};

#endif
