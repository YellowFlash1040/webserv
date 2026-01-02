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
#include "../FileUtils/FileUtils.hpp"
#include "../HttpMethod/HttpMethod.hpp"
#include "../HttpStatusCode/HttpStatusCode.hpp"
#include "CGIParser.hpp"

class RawResponse
{
	private:
	HttpStatusCode _statusCode;
	std::string _statusText;
	std::unordered_map<std::string, std::string> _headers;
	std::string _body;
	bool _isInternalRedirect;
	
	std::string _mimeType;      // Content type of file
	size_t _fileSize;    // file size (always set)
	FileUtils::FileDeliveryMode _fileMode = FileUtils::FileDeliveryMode::InMemory;
	std::string _filePath;      // Only used if streamed
	
	
	public:

	RawResponse();
	~RawResponse() = default;
	RawResponse(const RawResponse&) = default;
	RawResponse& operator=(const RawResponse&) = default;
	RawResponse(RawResponse&&) noexcept = default;
	RawResponse& operator=(RawResponse&&) noexcept = default;
	
	ResponseData toResponseData() const;
	void handleCgiScript();

	bool hasHeader(const std::string& key) const;
	bool isInternalRedirect() const;
	bool shouldClose() const;

	HttpStatusCode getStatusCode() const;
	const std::string& getStatusText() const;
	std::string getHeader(const std::string& key) const;
	const std::unordered_map<std::string, std::string>& getHeaders() const;
	const std::string& getBody() const;

	size_t getFileSize() const;
	const std::string& getFilePath() const;
	const std::string& getMimeType() const;
	
	void setStatusCode(HttpStatusCode code);
	void setDefaultHeaders();
	void addHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);

	void setInternalRedirect(bool flag);
	void setFileMode(FileUtils::FileDeliveryMode mode);
	void setFilePath(const std::string& path);
	void setMimeType(const std::string& mime);

	void setFileSize(size_t size);
	
	std::string getErrorPageUri(const std::map<HttpStatusCode, std::string>& error_pages,
                                HttpStatusCode status) const;
	void addErrorDetails(const RequestContext& ctx, HttpStatusCode code);
	void addDefaultError(HttpStatusCode code);

	bool parseFromCgiOutput(const std::string& cgiOutput);
	};

#endif
