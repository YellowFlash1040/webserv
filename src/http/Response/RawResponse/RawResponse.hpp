#ifndef RAWRESPONSE_HPP
#define RAWRESPONSE_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <fstream>

#include "../../Request/RawRequest/RawRequest.hpp"
#include "../../../config/Config/request_resolving/RequestContext/RequestContext.hpp"
// #include "../FileHandler/FileHandler.hpp"
#include "../ResponseData/ReposneData.hpp"

enum class FileDeliveryMode
{
	InMemory,
	Streamed
};

struct DeliveryInfo
{
	FileDeliveryMode mode;
	size_t size;
	
	DeliveryInfo(FileDeliveryMode m, size_t s) : mode(m), size(s) {}
};

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
	bool _isExternalRedirect;
	std::string _redirectTarget;   // URI for redirect
	
	FileDeliveryMode _fileMode = FileDeliveryMode::InMemory;
	std::string _filePath;      // Only used if streamed
	std::string _mimeType;      // Content type of file
	
	public:

	RawResponse() = default; //will be used for bad requests
	RawResponse(const RequestData& req, const RequestContext& ctx); 
	~RawResponse() = default;
	RawResponse(const RawResponse&) = default;
	RawResponse& operator=(const RawResponse&) = default;
	RawResponse(RawResponse&&) noexcept = default;
	RawResponse& operator=(RawResponse&&) noexcept = default;

	
	void setFileMode(FileDeliveryMode mode) { _fileMode = mode; }

	void setFilePath(const std::string& path) { _filePath = path; }

	void setMimeType(const std::string& mime) { _mimeType = mime; }


	// Getters
	HttpStatusCode getStatusCode() const;
	const std::string& getStatusText() const;
	const std::unordered_map<std::string, std::string>& getHeaders() const;
	const std::string& getBody() const;
	bool isInternalRedirect() const;
	const std::string& getRedirectTarget() const;
	

	// Setters
	void setStatus(HttpStatusCode code);
	void setDefaultHeaders();
	void addHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	void setRedirectTarget(const std::string& uri);
	void setInternalRedirect(bool flag);

	std::string codeToText(HttpStatusCode code);

	
	std::string allowedMethodsToString(const std::vector<HttpMethod>& allowed_methods);

	void addDefaultErrorDetails(HttpStatusCode code);
	bool shouldClose() const;
	
	void handleCgiScript();
	void handleExternalRedirect(const std::string& reqUri);
	
	static std::string httpMethodToString(HttpMethod method);
	
	void sendFile(const std::string &filePath, const std::string &mimeType);
	bool hasHeader(const std::string& key) const;
	std::string getHeader(const std::string& key) const;
	
	ResponseData toResponseData() const;
	void setFileContent(const std::string& content, const std::string& mimeType);
	void setFilePath(const std::string& path, const std::string& mimeType);
	
	FileDeliveryMode getFileMode() const;
	const std::string& getFilePath() const;
	const std::string& getMimeType() const;

	void setStatusCode(HttpStatusCode code);
	
	bool isExternalRedirect() const;
	
	};

#endif
