#ifndef RAWREQUEST_HPP
#define RAWREQUEST_HPP

#include <string>
#include <unordered_map>
#include <iostream>
#include <variant>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#include "../utils/StrUtils.hpp"
#include "../HttpMethod/HttpMethod.hpp"
#include "../RequestData/RequestData.hpp"
#include "../utils/UriUtils.hpp"
#include "debug.hpp"
#include "BodyParser.hpp"

enum BodyType
{
	NO_BODY, // No body present
	CHUNKED, // Transfer-Encoding: chunked
	SIZED,   // Content-Length specified
	ERROR    // Invalid or unexpected body state
};

class RawRequest
{
	private:
		// Properties
		std::string m_tempBuffer;
		std::string m_body;
		std::string m_chunkedBuffer;
		std::string m_conLenBuffer;
		HttpMethod m_method;
		std::string m_rawUri;
		std::string m_uri;
		std::string m_host;
		std::string m_query;
		std::string m_httpVersion;
		std::unordered_map<std::string, std::string> m_headers;
		BodyType m_bodyType;

		bool m_headersDone;
		bool m_terminatingZeroMet;
		bool m_bodyDone;
		bool m_requestDone;
		bool m_isBadRequest;
		bool m_shouldClose;

		// -----------------------------
		// Private Parsing Helpers
		// -----------------------------
		void handleHeaderPart();
		bool extractHeaderPart(std::string& headerPart);
		void parseRequestLineAndHeaders(const std::string& headerPart);
		void parseRequestLine(const std::string& firstLine);
		void splitUriAndQuery();
		void parseHeaders(std::istringstream& stream);
		void parseAndStoreHeaderLine(const std::string& line);
		void finalizeHeaders();
		void finalizeHeaderPart();
		void appendBodyBytes(const std::string& data);
		size_t getContentLengthValue() const;
		void appendToBody(const std::string& data);
		void setRequestDone();

	public:
		// Construction and destruction
		RawRequest();
		~RawRequest() = default;
		RawRequest(const RawRequest& other) = delete;
		RawRequest& operator=(const RawRequest& other) = delete;
		RawRequest(RawRequest&& other) noexcept = default;
		RawRequest& operator=(RawRequest&& other) noexcept = default;

		// -----------------------------
		// Public Parsing / Data Methods
		// -----------------------------
		bool parse();
		RequestData buildRequestData() const;

		bool isHeadersDone() const;
		bool isBodyDone() const;
		bool isRequestDone() const;
		bool isBadRequest() const;
		bool shouldClose() const;

		HttpMethod getMethod() const;
		const std::string& getUri() const;
		const std::string& getQuery() const;
		const std::string& getHttpVersion() const;
		const std::unordered_map<std::string, std::string>& getHeaders() const;
		const std::string getHeader(const std::string& name) const;
		std::string getHost() const;
		BodyType getBodyType() const;
		const std::string& getTempBuffer() const;
		const std::string& getBody() const;

		// -----------------------------
		// Public Setters / Modifiers
		// -----------------------------
		void setMethod(HttpMethod method);
		void setUri(const std::string& uri);
		void setHeadersDone();
		void addHeader(const std::string& name, const std::string& value);
		void setShouldClose(bool value);
		void setTempBuffer(const std::string& buffer);
		void setBody(const std::string& data);
		void appendTempBuffer(const std::string& data);
		void markBadRequest();
};

#endif
