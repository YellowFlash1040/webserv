#ifndef RAWREQUEST_HPP
#define RAWREQUEST_HPP

#include "../HttpMethod/HttpMethod.hpp"
#include "../RequestData/RequestData.hpp"
#include "debug.hpp"

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

class RawRequest
{
	private:
		std::string _tempBuffer;
		std::string _rlAndHeadersBuffer;
		std::string _body;
		std::string _chunkedBuffer;
		std::string _conLenBuffer;
		HttpMethod _method;
		std::string _rawUri;
		std::string _uri;
		std::string _query;  // optional query string (part after '?')
		std::string _httpVersion;
		std::unordered_map<std::string, std::string> _headers;
		BodyType _bodyType;
		std::string _errorMessage;
		
		// Parsing state
		bool _headersDone;
		bool _terminatingZeroMet;
		bool _bodyDone;
		bool _requestDone;
		bool _isBadRequest;
	
		// Helpers
		static std::string bodyTypeToString(BodyType t);
		static std::string fullyDecodePercent(const std::string& rawUri);
		static std::string decodePercentOnce(const std::string& s);
		static std::string normalizePath(const std::string& uri);
		
		size_t conLenValue() const;
		std::string decodeChunkedBody(size_t& bytesProcessed);
		void parseRequestLineAndHeaders(const std::string& headerPart);
		void appendToChunkedBuffer(const std::string& data);
		size_t remainingConLen() const;
		void consumeTempBuffer(size_t n);
		void appendToConLenBuffer(const std::string& data);
		void parseRequestLine(const std::string& firstLine);
		void parseHeaders(std::istringstream& stream);
		
		void appendToBody(const std::string& data);
		void addHeader(const std::string& name, const std::string& value);
		void setChunkedBuffer(std::string&& newBuffer);
		

	public:
		RawRequest();
		~RawRequest();
		RawRequest(const RawRequest& other);
		RawRequest& operator=(const RawRequest& other);
		RawRequest(RawRequest&& other) noexcept;
		RawRequest& operator=(RawRequest&& other) noexcept;

		
		const std::string getHeader(const std::string& name) const;
		HttpMethod getMethod() const;
		bool conLenReached() const;
		void setHeadersDone();
		void setBodyDone();
		bool isHeadersDone() const;
		bool isBodyDone() const;
		bool isRequestDone() const;
		const std::string& getTempBuffer() const;
		
		void setRequestDone();
		void setTempBuffer(const std::string& buffer);
		
		void appendBodyBytes(const std::string& data);
		void separateHeadersFromBody();
		void appendTempBuffer(const std::string& data);
		RequestData buildRequestData() const;
		
		void markBadRequest(const std::string& msg);
		bool isBadRequest() const;
		
		const std::string& getUri() const;
		std::string getHost() const;

		void printRequest(size_t idx = 0) const;
		
		void setMethod(HttpMethod method);
    	void setMethod(const std::string& method);
		void setUri(const std::string& uri);
		

	};

#endif
