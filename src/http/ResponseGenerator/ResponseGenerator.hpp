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
#include "../../cgi/CGI.hpp"
#include "../../cgi/CGIHandler.hpp"
#include "../utils/debug.hpp"
#include "../utils/StrUtils.hpp"
#include "UploadModule.hpp"
#include "../network/Client/Client.hpp"


namespace ResponseGenerator
{
    void genResponse(
        const RawRequest& rawReq,
        const Client& client,
        const RequestContext& ctx,
        RawResponse& rawResp
    );

	bool isMethodAllowed(
		HttpMethod method,
		const std::vector<HttpMethod>& allowed_methods
	);

    void processGet(
        RequestData& req,
        const Client& client,
        const RequestContext& ctx,
        RawResponse& resp
    );

    void processPost(
        RequestData& req,
        const Client& client,
        const RequestContext& ctx,
        RawResponse& resp
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