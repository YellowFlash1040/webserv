#ifndef RAWREQUEST_HPP
#define RAWREQUEST_HPP

#include <string>
#include <iostream>

#include "StrUtils.hpp"
#include "HttpMethod.hpp"
#include "RequestData.hpp"
#include "UriUtils.hpp"
#include "BodyParser.hpp"
#include "debug.hpp"

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

    // Accessors
    size_t contentLength() const;

    // Methods
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

    void appendToBody(const std::string& data);

  public:
    // Construction and destruction
    RawRequest();
    RawRequest(const RawRequest& other) = delete;
    RawRequest& operator=(const RawRequest& other) = delete;
    RawRequest(RawRequest&& other) noexcept = default;
    RawRequest& operator=(RawRequest&& other) noexcept = default;
    ~RawRequest() = default;

    // Accessors
    bool isHeadersDone() const;
    bool isBodyDone() const;
    bool isRequestDone() const;
    bool isBadRequest() const;
    bool shouldClose() const;
    HttpMethod method() const;
    const std::string& uri() const;
    const std::string& query() const;
    const std::string& httpVersion() const;
    const std::unordered_map<std::string, std::string>& headers() const;
    const std::string header(const std::string& name) const; // may return ""
    const std::string& host() const;
    BodyType bodyType() const;
    const std::string& tempBuffer() const;
    const std::string& body() const;
    void setMethod(HttpMethod method);
    void setUri(const std::string& uri);
    void setHeadersDone();
    void addHeader(const std::string& name, const std::string& value);
    void setShouldClose(bool value);
    void setTempBuffer(const std::string& buffer);
    void setBody(const std::string& data);
    void appendTempBuffer(const std::string& data);
    void setRequestDone();
    void markBadRequest();

    // Methods
    bool parse();
    RequestData buildRequestData() const;
};

#endif
