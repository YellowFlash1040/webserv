#ifndef PARSEDREQUEST_HPP
#define PARSEDREQUEST_HPP

#include "../HttpMethod/HttpMethod.hpp"
#include <string>
#include <unordered_map>
#include <iostream>
#include <variant>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define ORANGE "\033[38;5;214m"
#define RED "\033[31m"
#define MINT "\033[38;2;150;255;200m"
#define RESET "\033[0m"

enum class BodyType
{
	NO_BODY,   // GET, HEAD, etc.
	CHUNKED,  // Transfer-Encoding: chunked
	SIZED,    // Content-Length specified
	ERROR     // Invalid or unexpected body state
};

class ParsedRequest
{
	private:
		// Buffers
		std::string _tempBuffer;
		std::string _rlAndHeadersBuffer;
		std::string _body;
		std::string _chunkedBuffer;
		std::string _contentLengthBuffer;
		std::string _method;
		HttpMethodEnum _methodEnum;
		std::string _uri;
		std::string _httpVersion;
		std::unordered_map<std::string, std::string> _headers;
		BodyType _bodyType;
		
		// Parsing state
		bool _headersDone;
		bool _terminatingZero;
		bool _bodyDone;
		bool _needResp;
		bool _requestDone;

		// Message framing
		int  _contentLength;

		// Helpers
		static void removeCarriageReturns(std::string& str);
		static void trimLeadingWhitespace(std::string& str);
		static bool iequals(const std::string& a, const std::string& b);
		static HttpMethodEnum stringToHttpMethod(const std::string& method);
		
	public:
		ParsedRequest();
		~ParsedRequest();
		ParsedRequest(const ParsedRequest& other);
		ParsedRequest& operator=(const ParsedRequest& other);
		ParsedRequest(ParsedRequest&& other) noexcept;
		ParsedRequest& operator=(ParsedRequest&& other) noexcept;

		const std::string& getRlAndHeadersBuffer() const;
		const std::string getHeader(const std::string& name) const;
		BodyType getBodyType() const;
		const std::string& getMethod() const;
		const std::string& getUri() const;
		const std::string getHost() const;
		const std::string& getHttpVersion() const;
		const std::string& getBody() const;
		const std::string& getChunkedBuffer() const;
		const std::string& getContentLengthBuffer() const;
		const std::string& getTempBuffer() const;
		size_t getContentLength() const;
		
		bool isRequestDone() const;
		bool isHeadersDone() const;
		bool isBodyDone() const;
		bool isTerminatingZero();
		bool isContentLengthComplete() const;
		bool needsResponse() const;
		
		void setRlAndHeadersBuffer(const std::string& newBuf);
		void setMethod(const std::string& m);
		void setUri(const std::string& u);
		void setHttpVersion(const std::string& v);
		void setBody(const std::string& body);
		void setTempBuffer(const std::string& buffer);
		
		void setHeadersDone();
		void setBodyDone();
		void setRequestDone();
		void setTerminatingZero();
		void setBodyType(BodyType type);
		void setChunkedBuffer(std::string&& newBuffer);
		void setResponseAdded();
		
		size_t remainingContentLength() const;
		
		void appendToRlAndHeaderBuffer(const std::string& data);
		void appendTobody(const std::string& data);
		void appendTempBuffer(const std::string& data);
		void appendToBody(const std::string& data);
		
		void addHeader(const std::string& name, const std::string& value);
		
		void clearTempBuffer();
		void clearChunkedBuffer();
		void consumeTempBuffer(size_t n);
		
		size_t extractContentLength() const;
		std::string decodeChunkedBody(size_t& bytesProcessed);
		void parseRequestLineAndHeaders(const std::string& headerPart);
		void appendToChunkedBuffer(const std::string& data);
		
		void appendToContentLengthBuffer(const std::string& data);
		void clearContentLengthBuffer();
		void parseRequestLine(const std::string& firstLine);
		void parseHeaders(std::istringstream& stream);
		void separateHeadersFromBody();
		void appendBodyBytes(const std::string& data);
		
		static std::string bodyTypeToString(BodyType t);
};

#endif
