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
		// Properties
		HttpStatusCode m_statusCode;
		std::string m_statusText;
		std::unordered_map<std::string, std::string> m_headers;
		std::string m_body;
		bool m_isInternalRedirect;
		std::string m_mimeType;
		size_t m_fileSize;

	public:
		// Construction and destruction
		RawResponse();
		~RawResponse() = default;
		RawResponse(const RawResponse&) = default;
		RawResponse& operator=(const RawResponse&) = default;
		RawResponse(RawResponse&&) noexcept = default;
		RawResponse& operator=(RawResponse&&) noexcept = default;
		
		// Accessors
		bool hasHeader(const std::string& key) const;
		bool isInternalRedirect() const;
		bool shouldClose() const;
		HttpStatusCode statusCode() const;
		const std::string& statusText() const;
		std::string header(const std::string& key) const;
		const std::unordered_map<std::string, std::string>& headers() const;
		const std::string& body() const;
		size_t fileSize() const;
		const std::string& mimeType() const;
		void setStatusCode(HttpStatusCode code);
		void setBody(const std::string& body);
		void setInternalRedirect(bool flag);
		void setMimeType(const std::string& mime);
		void setFileSize(size_t size);
		
		// Methods
		void addDefaultHeaders();
		void addHeader(const std::string& key, const std::string& value);
		std::string lookupErrorPageUri(const std::map<HttpStatusCode, std::string>& error_pages,
									HttpStatusCode status) const;
		void addErrorDetails(const RequestContext& ctx, HttpStatusCode code);
		void addDefaultError(HttpStatusCode code);
		ResponseData toResponseData() const;
		void handleCgiScript();
		bool parseFromCgiOutput(const std::string& cgiOutput);
	};

#endif
