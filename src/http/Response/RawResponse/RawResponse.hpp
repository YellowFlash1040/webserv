#ifndef RAWRESPONSE_HPP
#define RAWRESPONSE_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <fstream>

#include "../../Request/RawRequest/RawRequest.hpp"
#include "../../../config/Config/request_resolving/RequestContext/RequestContext.hpp"
#include "../ResponseData/ResponseData.hpp"
#include "../FileDeliveryMode/FileDeliveryMode.hpp"
#include "../HttpMethod/HttpMethod.hpp"
#include "../HttpStatusCode/HttpStatusCode.hpp"

class RawResponse
{
	private:
	HttpStatusCode _statusCode;
	std::string _statusText;
	std::unordered_map<std::string, std::string> _headers;
	std::string _body;
	bool _isInternalRedirect;
	bool _isExternalRedirect;
	std::string _redirectTarget;   // URI for redirect
	
	FileDeliveryMode _fileMode = FileDeliveryMode::InMemory;
	std::string _filePath;      // Only used if streamed
	size_t _fileSize;    // file size (always set)
	std::string _mimeType;      // Content type of file
	
	public:

	RawResponse();
	
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
	void setExternalRedirect(bool flag);

	void addDefaultError(HttpStatusCode code);
	bool shouldClose() const;
	
	void handleCgiScript();

	
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
	void setFileSize(size_t size);
	size_t getFileSize() const;
	
	};

#endif
