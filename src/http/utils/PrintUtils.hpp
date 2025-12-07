#ifndef PRINTUTILS_HPP
#define PRINTUTILS_HPP

#include <iostream>
#include "../config/Config/request_resolving/RequestContext/RequestContext.hpp"

#include "debug.hpp"
#include "RawRequest.hpp"
#include "../../ConnectionManager/ClientState/ClientState.hpp"


namespace PrintUtils
{
    void printReqContext(const RequestContext& ctx);
	void printRawRequest(const RawRequest& req, size_t idx);
	void printAllResponses(const ClientState& clientState);
}

#endif