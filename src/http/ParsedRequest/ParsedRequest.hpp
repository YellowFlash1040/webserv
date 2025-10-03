#ifndef PARSEDREQUEST_HPP
#define PARSEDREQUEST_HPP

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
		bool _chunked;

		// Helpers
		static void removeCarriageReturns(std::string& str);
		static void trimLeadingWhitespace(std::string& str);

	public:
		// Canonical form
		ParsedRequest();
		~ParsedRequest();
		ParsedRequest(const ParsedRequest& other);
		ParsedRequest& operator=(const ParsedRequest& other);
		ParsedRequest(ParsedRequest&& other) noexcept;
		ParsedRequest& operator=(ParsedRequest&& other) noexcept;

		// Getters
		const std::string& getMethod() const;
		const std::string& getUri() const;
		const std::string& getHttpVersion() const;
		std::string getHeader(const std::string& name) const;
		const std::string& getRlAndHeadersBuffer() const;
		const std::string& getBody() const;
		size_t getContentLength() const;
		bool isRequestDone() const;
		bool isHeadersDone() const;
		bool isBodyDone() const;

		// Setters
		void setMethod(const std::string& m);
		void setUri(const std::string& u);
		void setHttpVersion(const std::string& v);
		void addHeader(const std::string& name, const std::string& value);
		void setHeadersDone();
		void setBodyDone();
		void setRequestDone();


		void setRlAndHeadersBuffer(const std::string& newBuf);

		// Buffer manipulation
		void appendToRlAndHeaderBuffer(const std::string& data);
		void appendTobody(const std::string& data);

		void clearRlAndHeaderBuffer();
		void clearbody();

		// Parsing / Body handling
		bool headersParsed() const;
		size_t extractContentLength() const;
		
		std::string decodeChunkedBody(size_t& bytesProcessed);
		void parseRequestLineAndHeaders(const std::string& headerPart);
		bool iequals(const std::string& a, const std::string& b) const;
		void clearRlAndHeadersBuffer();
		
		void setBody(const std::string& body);
		void setTerminatingZero();
		std::string& getChunkedBuffer();
		void appendToChunkedBuffer(const std::string& data);
		void clearChunkedBuffer();
		
		bool contentLengthComplete() const;
		const std::string& getContentLengthBuffer() const;
		void appendToContentLengthBuffer(const std::string& data);
		void clearContentLengthBuffer();
		
		BodyType getBodyType() const;
		void setBodyType(BodyType type);
		
		void parseRequestLine(const std::string& firstLine);
		void parseHeaders(std::istringstream& stream);
		
		bool isTerminatingZero();
		
		std::string& getTempBuffer();
		void setTempBuffer(const std::string& buffer);
		void appendTempBuffer(const std::string& data);
		void clearTempBuffer();
		
		std::string& getContentLengthBuffer();
		
    	bool needsResponse() const;
    	void setNeedsResp(bool needsResp);
		void setResponseAdded();
		
		size_t getRemainingContentLength() const;
		void consumeTempBuffer(size_t n);
		void appendToBody(const std::string& data);

		void setChunkedBuffer(std::string&& newBuffer);

};

#endif
