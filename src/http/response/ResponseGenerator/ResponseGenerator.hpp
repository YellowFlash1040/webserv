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
#include "CgiRequestResult.hpp"
#include "FileReader.hpp"

namespace ResponseGenerator
{
    // Main entry point
    void genResponse(
        const RawRequest& rawReq,
        const RequestContext& ctx,
        RawResponse& rawResp,
        CgiRequestResult& cgiResult
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
        const RequestData& req,
        const RequestContext& ctx,
        RawResponse& resp,
        CgiRequestResult& cgiResult
    );

    void processPost(
        const RequestData& req,
        const RequestContext& ctx,
        RawResponse& resp,
        CgiRequestResult& cgiResult
    );

	void processDelete(
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
		const std::string& reqUri,
		RawResponse& rawResp
	);

	void fillSuccessfulResponse(
		RawResponse& resp,
		const std::string& filePath
	);

	void fillAutoindexResponse(
		RawResponse& resp,
		const std::string& dirPath
	);

    void setConnectionHeader(const RawRequest& rawReq, RawResponse& rawResp);

    // Status codes
    void handleNotFound(const RequestContext& ctx, RawResponse& rawResp);
    void handleNoPermission(const RequestContext& ctx, RawResponse& rawResp);
    void handleServerError(const RequestContext& ctx, RawResponse& rawResp);
    void handlePayloadTooLarge(const RequestContext& ctx, RawResponse& rawResp);
    void handleBadGateway(const RequestContext& ctx, RawResponse& rawResp);
    void handleBadRequest(const RequestContext& ctx, RawResponse& rawResp);
    void handleMethodNotAllowed(const RequestContext& ctx, RawResponse& rawResp);

    // Static file
    void handleStaticFile(const RequestContext& ctx, RawResponse& rawResp, const FileInfo& file);
    void serveStaticFile(const RequestContext& ctx, RawResponse& rawResp);
    
    // Directory
    void handleDirectory(const RequestContext& ctx, RawResponse& rawResp);
    void serveIndexFile(const std::string& indexPath, const RequestContext& ctx,
                        RawResponse& rawResp);
    void generateAutoIndex(const RequestContext& ctx, RawResponse& rawResp);

    // CGI
    void handleCGI(const RequestData& req, const RequestContext& ctx,
               RawResponse& rawResp, CgiRequestResult& cgiResult, const std::string& ext);

    HttpStatusCode checkScriptValidity(const std::string& scriptPath);
    void handleScriptInvalidity(HttpStatusCode status, const RequestContext& ctx,
                            RawResponse& rawResp);
}

#endif