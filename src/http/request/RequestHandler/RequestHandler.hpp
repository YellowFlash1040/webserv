#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "ResponseGenerator.hpp"
#include "Config.hpp"
#include "RequestContext.hpp"
#include "Client.hpp"
#include "CgiRequestResult.hpp"

namespace RequestHandler
{
	RawResponse handleSingleRequest(const RawRequest& rawReq,
									const Client& client,
									const Config& config,
									CgiRequestResult& cgiResult);

}

#endif