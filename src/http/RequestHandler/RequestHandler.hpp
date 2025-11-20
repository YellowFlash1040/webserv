# ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

#include "../Response/RawResponse/RawResponse.hpp"
#include "../ConnectionManager/ClientState/ClientState.hpp"
#include "../../network/NetworkEndpoint/NetworkEndpoint.hpp"
#include "../Request/RawRequest/RawRequest.hpp"
#include "../Request/RequestData/RequestData.hpp"
#include "../../config/Config/request_resolving/RequestContext/RequestContext.hpp"
#include "../HttpStatusCode/HttpStatusCode.hpp"
#include "../HttpMethod/HttpMethod.hpp"
#include "../Response/FileHandler/FileHandler.hpp"
#include "CGI.hpp"
#include "debug.hpp"

class RequestHandler
{
	public:
	RequestHandler(ClientState& client) : clientState(client) {}
	
	void processRequest(RawRequest& rawReq, const NetworkEndpoint& endpoint, const RequestContext& ctx);
	std::string getErrorPageUri(const RequestContext& ctx, HttpStatusCode status) const;
	void enqueueErrorResponse(const RequestContext& ctx, HttpStatusCode status);
		static std::string httpMethodToString(HttpMethod method);
	
	private:
	ClientState& clientState; // Needed to enqueue response
	
	
	bool isMethodAllowed(HttpMethod method, const std::vector<HttpMethod>& allowed_methods);
	
	void processGet(RequestData& req, const NetworkEndpoint& endpoint, const RequestContext& ctx, RawResponse& resp);
	void processPost(RequestData& req, const NetworkEndpoint& endpoint, const RequestContext& ctx, RawResponse& resp);
	void processDelete(RequestData& req, const NetworkEndpoint& endpoint, const RequestContext& ctx, RawResponse& resp);
	
	std::string getCgiPathFromUri(const std::string& uri, const std::map<std::string, std::string>& cgi_pass,
		HttpStatusCode& outStatus);
	void setFileDelivery(RawResponse& resp, const std::string& path, FileHandler& fileHandler);
	std::string readFileToString(const std::string& path);
	
	void serveBadRequest(RawRequest& rawReq, const NetworkEndpoint& endpoint, const RequestContext& ctx);
	void addGeneralErrorDetails(RawResponse& resp, const RequestContext& ctx, HttpStatusCode code);
	
	std::string handleCGI(RequestData& req,
									  const NetworkEndpoint& endpoint,
									  const std::string& interpreter,
									  const std::string& scriptPath);
	void processUpload(RequestData &req, const RequestContext &ctx, RawResponse &resp);

};

#endif