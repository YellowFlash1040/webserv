#ifndef PRINTUTILS_HPP
#define PRINTUTILS_HPP

#include <iostream>

#include "RequestContext.hpp"
#include "RawRequest.hpp"
#include "ClientState.hpp"
#include "debug.hpp"


namespace PrintUtils
{
    void printReqContext(const RequestContext& ctx);
	void printRawRequest(const RawRequest& req);
	void printAllResponses(const ClientState& clientState);
}

#endif