#ifndef PARSEDREQUEST_HPP
#define PARSEDREQUEST_HPP

#include <string>
#include <unordered_map>
#include <iostream>
#include <variant>

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define ORANGE "\033[38;5;214m"
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
		std::string _bodyBuffer;
		std::string _chunkedBuffer;
		std::string _contentLengthBuffer;
		std::string _method;
		std::string _uri;
		std::string _httpVersion;
		std::unordered_map<std::string, std::string> _headers;
		BodyType _bodyType;
		std::string _body;
		
		// Parsing state
		bool _headersDone;
		bool _terminatingZero;
		bool _bodyDone;
		bool _requestDone;
		bool _waitingForMoreData;
		

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
		std::string& getBodyBuffer();
		size_t getContentLength() const;
		bool isRequestDone() const;
		bool isHeadersDone() const;
		bool isBodyDone() const;
		bool isChunked() const;

		// Setters
		void setMethod(const std::string& m);
		void setUri(const std::string& u);
		void setHttpVersion(const std::string& v);
		void addHeader(const std::string& name, const std::string& value);
		void setHeadersDone();
		void setBodyDone();
		void setRequestDone();
		void setContentLength(int value);
		void setChunked(bool value);
		void setRlAndHeadersBuffer(const std::string& newBuf);

		// Buffer manipulation
		void appendToRlAndHeaderBuffer(const std::string& data);
		void appendToBodyBuffer(const std::string& data);
		void appendToBuffers(const std::string& data);
		void clearRlAndHeaderBuffer();
		void clearBodyBuffer();

		// Parsing / Body handling
		bool headersParsed() const;
		size_t extractContentLength() const;
		bool bodyComplete() const;
		
		std::string decodeChunkedBody();
		void parseRequestLineAndHeaders(const std::string& headerPart);
		bool iequals(const std::string& a, const std::string& b) const;
		void clearRlAndHeadersBuffer();
		
		const std::string& getBody() const;
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


	
	};

#endif
