#ifndef RAWREQUEST_HPP
#define RAWREQUEST_HPP

#include <string>
#include <unordered_map>
#include <sstream>
#include <stdexcept>

#include "../utils/StrUtils.hpp"
#include "../HttpMethod/HttpMethod.hpp"
#include "../RequestData/RequestData.hpp"
#include "../../BodyType/BodyType.hpp"
#include "../utils/UriUtils.hpp"
#include "debug.hpp"
#include "BodyParser.hpp"

class RawRequest
{
	public:
		RawRequest();
		~RawRequest() = default;
		RawRequest(const RawRequest&) = delete;
		RawRequest& operator=(const RawRequest&) = delete;
		RawRequest(RawRequest&&) noexcept = default;
		RawRequest& operator=(RawRequest&&) noexcept = default;

		bool parse();
		void separateHeadersFromBody();
		void appendBodyBytes(const std::string& data);

		// ---- request state ----
		bool isHeadersDone() const;
		bool isBodyDone() const;
		bool isRequestDone() const;
		bool isBadRequest() const;
		bool shouldClose() const;

		// ---- body helpers ----
		size_t getContentLengthValue() const;
		const std::string& getBody() const;

		// ---- accessors ----
		const std::string& getTempBuffer() const;
		const std::string& getUri() const;
		std::string getHost() const;
		HttpMethod getMethod() const;
		const std::string& getHttpVersion() const;
		BodyType::Type getBodyType() const;
		const std::string getHeader(const std::string& name) const;
		const std::unordered_map<std::string, std::string>& getHeaders() const;
		
		// ---- mutation ----
		void setMethod(HttpMethod method);
		void setGetMethod();
		void setUri(const std::string& uri);
		void setShouldClose(bool value);
		void setHeadersDone();
		void setRequestDone();
		void markBadRequest();
		void addHeader(const std::string& name, const std::string& value);
		void appendTempBuffer(const std::string& data);
		void setTempBuffer(const std::string& buffer);

		// ---- finalization ----
		RequestData buildRequestData() const;

	private:
		// ---- buffers
		std::string _tempBuffer;
		std::string _rlAndHeadersBuffer;
		std::string _body;
		std::string _chunkedBuffer;
		std::string _conLenBuffer;

		// ---- request metadata ----
		HttpMethod _method;
		std::string _rawUri;
		std::string _uri;
		std::string _host;
		std::string _query;
		std::string _httpVersion;
		std::unordered_map<std::string, std::string> _headers;
		BodyType::Type _bodyType;

		// ---- parsing state ----
		bool _headersDone;
		bool _terminatingZeroMet;
		bool _bodyDone;
		bool _requestDone;
		bool _isBadRequest;
		bool _shouldClose;

		// ---- INTERNAL helpers
		void parseRequestLineAndHeaders(const std::string& headerPart);
		void parseRequestLine(const std::string& firstLine);
		void parseHeaders(std::istringstream& stream);
		void processHeaderLine(const std::string& line);
		void finalizeHeaderParsing();
		std::string extractHost(const std::string& hostHeader) const;

		// ---- body ownership helpers ----
		void appendToBody(const std::string& data);
};

#endif
