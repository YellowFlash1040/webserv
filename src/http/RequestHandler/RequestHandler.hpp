#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../ResponseGenerator/ResponseGenerator.hpp"
#include "Config.hpp"
#include "../../config/Config/request_resolving/RequestContext/RequestContext.hpp"
#include "../network/Client/Client.hpp"
#include "RequestResult.hpp"

namespace RequestHandler
{
	RawResponse handleSingleRequest(const RawRequest& rawReq,
									const Client& client,
									const Config& config,
									RequestResult& result);

	void handleInternalRedirect(const RawRequest& rawReq,
								const Client& client,
								const RequestContext& ctx,
								const RawResponse& curRawResp,
								RawResponse& redirResp,
								RequestResult& result);
}

#endif