#pragma once

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

# include "../ResponseGenerator/ResponseGenerator.hpp"
# include "Config.hpp"
# include "../../config/Config/request_resolving/RequestContext/RequestContext.hpp"

namespace RequestHandler
{

RawResponse handleSingleRequest(const RawRequest& rawReq,
                                const NetworkEndpoint& endpoint,
                                const Config& config);

void handleInternalRedirect(const RawRequest& rawReq,
                            const NetworkEndpoint& endpoint,
                            const RequestContext& ctx,
                            const RawResponse& curRawResp,
                            RawResponse& redirResp);

} // namespace RequestHandler

#endif
