#ifndef RESPONSEGENERATOR_HPP
#define RESPONSEGENERATOR_HPP

#include "../Response/RawResponse/RawResponse.hpp"
#include "../ConnectionManager/ClientState/ClientState.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "../Request/RawRequest/RawRequest.hpp"
#include "../Request/RequestData/RequestData.hpp"
#include "../../config/Config/request_resolving/RequestContext/RequestContext.hpp"
#include "../HttpStatusCode/HttpStatusCode.hpp"
#include "../HttpMethod/HttpMethod.hpp"
#include "../FileUtils/FileUtils.hpp"
#include "debug.hpp"
#include "../utils/StrUtils.hpp"
#include "UploadModule.hpp"
#include "../network/Client/Client.hpp"
#include "RequestResult.hpp"


namespace ResponseGenerator
{
    // Main entry point
    void genResponse(
        const RawRequest& rawReq,
        const Client& client,
        const RequestContext& ctx,
        RawResponse& rawResp,
        RequestResult& result
    );

	bool isMethodAllowed(
		HttpMethod method,
		const std::vector<HttpMethod>& allowed_methods
	);

    void addAllowHeader(
        RawResponse& rawResp,
        const std::vector<HttpMethod>& allowed_methods
    );

    void processGet(
        RequestData& req,
        const Client& client,
        const RequestContext& ctx,
        RawResponse& resp,
        RequestResult& result
    );

    void processPost(
        RequestData& req,
        const Client& client,
        const RequestContext& ctx,
        RawResponse& resp,
        RequestResult& result
    );

	void processDelete(
		RequestData& req,
		const Client& client,
		const RequestContext& ctx,
		RawResponse& resp
	);

	std::string getCgiPathFromUri(
		const std::string& uri,
		const std::map<std::string, std::string>& cgi_pass,
		HttpStatusCode& outStatus
	);
	
	bool checkScriptValidity(
		const std::string& scriptPath,
		RawResponse& rawResp, 
		const RequestContext& ctx
	);

	void handleExternalRedirect(
		const RequestContext& ctx,
		std::string& reqUri,
		RawResponse& rawResp
	);

	void processUpload(
		RequestData &req,
		const RequestContext &ctx,
		RawResponse &resp
	);

	void fillSuccessfulResponse(
		RawResponse& resp,
		const std::string& filePath
	);

	void fillAutoindexResponse(
		RawResponse& resp,
		const std::string& dirPath
	);
}

#endif