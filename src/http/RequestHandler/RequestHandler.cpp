#include "RequestHandler.hpp"

namespace RequestHandler
{
	void handleExternalRedirect(const RawRequest& rawReq,
								const NetworkEndpoint& endpoint,
								const RequestContext& ctx,
								const RawResponse& curRawResp,
								RawResponse& redirResp)
	{
		if (!FileUtils::existsAndIsFile(ctx.resolved_path) || access(ctx.resolved_path.c_str(), R_OK) != 0)
		{
			redirResp.addDefaultError(curRawResp.getStatusCode());
		}
		else
		{
			RawRequest dummyReq;
			dummyReq.setGetMethod();
			dummyReq.setUri(ctx.resolved_path);
			dummyReq.setShouldClose(rawReq.shouldClose());

			ResponseGenerator::genResponse(dummyReq, endpoint, ctx, redirResp);
			redirResp.setStatusCode(curRawResp.getStatusCode());
		}
	}

	RawResponse handleSingleRequest(const RawRequest& rawReq,
									 const NetworkEndpoint& endpoint,
									 const Config& config)
	{
		RequestContext ctx = config.createRequestContext(endpoint, rawReq.getHost(), rawReq.getUri());
		RawResponse curRawResp;

		if (rawReq.shouldClose())
			curRawResp.addHeader("Connection", "close");

		ResponseGenerator::genResponse(rawReq, endpoint, ctx, curRawResp);

		if (curRawResp.isInternalRedirect())
		{
			std::string newUri = curRawResp.getErrorPageUri(ctx.error_pages, curRawResp.getStatusCode());
			RequestContext newCtx = config.createRequestContext(endpoint, rawReq.getHost(), newUri);
			RawResponse redirResp;

			handleExternalRedirect(rawReq, endpoint, newCtx, curRawResp, redirResp);
			return redirResp;
		}

		return curRawResp;
	}
}

