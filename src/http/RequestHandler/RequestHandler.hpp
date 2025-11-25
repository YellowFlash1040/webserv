#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../ResponseGenerator/ResponseGenerator.hpp"
#include "Config.hpp"
#include "../../config/Config/request_resolving/RequestContext/RequestContext.hpp"

namespace RequestHandler
{
	RawResponse handleSingleRequest(const RawRequest& rawReq,
									const NetworkEndpoint& endpoint,
									const Config& config);
									
	void handleExternalRedirect(const RequestContext& newCtx,
									std::string& newUri,
									RawResponse& redirResp);
									
}

#endif