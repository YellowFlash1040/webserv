#include "RequestHandler.hpp"

namespace RequestHandler
{
	void handleExternalRedirect(const RawRequest& rawReq,
								const Client& client,
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
			dummyReq.setMethod(HttpMethod::GET);
			dummyReq.setUri(ctx.resolved_path);
			dummyReq.setShouldClose(rawReq.shouldClose());

			ResponseGenerator::genResponse(dummyReq, client, ctx, redirResp);
			redirResp.setStatusCode(curRawResp.getStatusCode());
		}
	}

	RawResponse handleSingleRequest(const RawRequest& rawReq,
									 const Client& client,
									 const Config& config)
	{
		RequestContext ctx = config.createRequestContext(client.getListeningEndpoint(), rawReq.getHost(), rawReq.getUri());
		RawResponse curRawResp;

		if (rawReq.shouldClose())
			curRawResp.addHeader("Connection", "close");

		ResponseGenerator::genResponse(rawReq, client, ctx, curRawResp);

		if (curRawResp.isInternalRedirect())
		{
			std::string newUri = curRawResp.getErrorPageUri(ctx.error_pages, curRawResp.getStatusCode());
			RequestContext newCtx = config.createRequestContext(client.getListeningEndpoint(), rawReq.getHost(), newUri);
			RawResponse redirResp;

			handleExternalRedirect(rawReq, client, newCtx, curRawResp, redirResp);
			return redirResp;
		}

		return curRawResp;
	}
}

