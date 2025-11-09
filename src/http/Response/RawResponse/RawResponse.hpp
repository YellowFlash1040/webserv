#ifndef RAWRESPONSE_HPP
#define RAWRESPONSE_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <fstream>

#include "../../Request/RawRequest/RawRequest.hpp"
#include "../../../config/Config/request_resolving/RequestContext/RequestContext.hpp"

// #include "../Handlers/CgiHandler/CgiRequestData.hpp"
// #include "../Handlers/StaticHandler/StaticHandler.hpp"
#include "../FileHandler/FileHandler.hpp"
#include "../ResponseData/ReposneData.hpp"

class RawResponse
{
private:
	RequestData _req;
    RequestContext _ctx;
	
	HttpStatusCode _statusCode;
	std::string _statusText;
	std::unordered_map<std::string, std::string> _headers;
	std::string _body;
	bool _isInternalRedirect;
	
public:

	RawResponse() = default; //will be used for bad requests
	RawResponse(const RequestData& req, const RequestContext& ctx); 
   ~RawResponse() = default;
    RawResponse(const RawResponse&) = default;
    RawResponse& operator=(const RawResponse&) = default;
    RawResponse(RawResponse&&) noexcept = default;
    RawResponse& operator=(RawResponse&&) noexcept = default;

	// // Reset
	// void reset();

	// Getters
	HttpStatusCode getStatusCode() const;
	const std::string& getStatusText() const;
	const std::unordered_map<std::string, std::string>& getHeaders() const;
	const std::string& getBody() const;
	bool hasHeader(const std::string& key) const;

	// Setters
	void setStatus(HttpStatusCode code);
	void setDefaultHeaders();
	void addHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);

	// Map HTTP status code to default text
	std::string codeToText(HttpStatusCode code);
	void genResp();
	
	bool isMethodAllowed(HttpMethod method, const std::vector<HttpMethod>& allowed_methods);
	std::string allowedMethodsToString(const std::vector<HttpMethod>& allowed_methods);

	void generateDefaultErrorPage(HttpStatusCode code);
	bool shouldClose() const;
	
	void handleCgiScript();
	void handleRedirection();
	
	void handleStatic();

	static std::string httpMethodToString(HttpMethod method);
	
	ResponseData toResponseData() const;
	
	bool isInternalRedirect() const;

	};

#endif
