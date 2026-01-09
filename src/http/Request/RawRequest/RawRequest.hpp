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
#include "../../BodyType/BodyType.hpp"
#include "../utils/UriUtils.hpp" 
#include "debug.hpp"

class RawRequest
{
	public:
	RawRequest();
	~RawRequest() = default;
	RawRequest(const RawRequest& other) = delete;
	RawRequest& operator=(const RawRequest& other) = delete;
	RawRequest(RawRequest&& other) noexcept = default;
	RawRequest& operator=(RawRequest&& other) noexcept = default;
	
	bool parse();
	void markBadRequest();
	void appendBodyBytes(const std::string& data);
	void separateHeadersFromBody();
	void appendTempBuffer(const std::string& data);
	RequestData buildRequestData() const;
	
	bool isHeadersDone() const;
	bool isBodyDone() const;
	bool isRequestDone() const;
	bool isBadRequest() const;
	bool shouldClose() const;
	bool conLenReached() const;
	
	size_t getContentLengthValue() const;
	const std::string& getTempBuffer() const;
	const std::string getHeader(const std::string& name) const;
	const std::string& getUri() const;
	std::string getHost() const;
	HttpMethod getMethod() const;
	const std::string& getHttpVersion() const;
	const std::string& getBody() const;
	BodyType::Type getBodyType() const;
	const std::unordered_map<std::string, std::string>& getHeaders() const;
	
	void setMethod(HttpMethod method);
	void setUri(const std::string& uri);
	void setRequestDone();
	void setTempBuffer(const std::string& buffer);
	void setShouldClose(bool value);
	void setHeadersDone();
	void setBodyDone();
	void setChunkedBuffer(std::string&& newBuffer);
	void addHeader(const std::string& name, const std::string& value);

	private:
	std::string _tempBuffer;
	std::string _body;
	std::string _chunkedBuffer;
	std::string _conLenBuffer;
	HttpMethod _method;
	std::string _rawUri;
	std::string _uri;
	std::string _host;
	std::string _query;
	std::string _httpVersion;
	std::unordered_map<std::string, std::string> _headers;
	BodyType::Type _bodyType;
	
	// Parsing state
	bool _headersDone;
	bool _terminatingZeroMet;
	bool _bodyDone;
	bool _requestDone;
	bool _isBadRequest;
	bool _shouldClose;

	std::string decodeChunkedBody(size_t& bytesProcessed);
	void parseRequestLineAndHeaders(const std::string& headerPart);
	void appendToChunkedBuffer(const std::string& data);
	size_t remainingConLen() const;
	void consumeTempBuffer(size_t n);
	void appendToConLenBuffer(const std::string& data);
	void parseRequestLine(const std::string& firstLine);
	void parseHeaders(std::istringstream& stream);
	void parseSizedBody(const std::string& data);
	void parseChunkedBody();
	void appendToBody(const std::string& data);
	std::string extractHost(const std::string& hostHeader) const;

};

#endif
