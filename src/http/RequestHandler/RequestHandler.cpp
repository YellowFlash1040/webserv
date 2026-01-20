#include "RequestHandler.hpp"

namespace RequestHandler
{
	void handleInternalRedirect(const RawRequest& rawReq,
								const RequestContext& ctx,
								const RawResponse& curRawResp,
								RawResponse& redirResp,
								CgiRequestResult& cgiResult)
	{
        FileInfo path = FileUtils::getFileInfo(ctx.resolved_path);
		if (!(path.exists && path.isFile && path.readable))
		{
			redirResp.addDefaultError(curRawResp.statusCode());
		}
		else
		{
			RawRequest dummyReq;
			dummyReq.setMethod(HttpMethod::GET);
			dummyReq.setUri(ctx.resolved_path);
			dummyReq.setShouldClose(rawReq.shouldClose());

			ResponseGenerator::genResponse(dummyReq, ctx, redirResp, cgiResult);
			redirResp.setStatusCode(curRawResp.statusCode());
			ResponseGenerator::addAllowHeader(redirResp, ctx.allowed_methods);
		}
	}

	RawResponse handleSingleRequest(const RawRequest& rawReq,
									 const Client& client,
									 const Config& config,
									CgiRequestResult& cgiResult)
	{
		RequestContext ctx = config.createRequestContext(client.getListeningEndpoint(), rawReq.host(), rawReq.uri());
		RawResponse curRawResp;

		ResponseGenerator::genResponse(rawReq, ctx, curRawResp, cgiResult);

		if (curRawResp.isInternalRedirect())
		{
			std::string newUri = curRawResp.lookupErrorPageUri(ctx.error_pages, curRawResp.statusCode());
			RequestContext newCtx = config.createRequestContext(client.getListeningEndpoint(), rawReq.host(), newUri);
			RawResponse redirResp;

			handleInternalRedirect(rawReq, newCtx, curRawResp, redirResp, cgiResult);

			return redirResp;
		}

		return curRawResp;
	}
}

