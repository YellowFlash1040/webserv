#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <stdexcept>

class HttpRequest
{
	private:
		std::string _method;
		std::string _uri;
		std::string _httpVersion;
		std::map<std::string, std::string> _headers;
		std::string _body;
		//queryParams ?
		//cookies?
		
	public:
		HttpRequest();
		~HttpRequest();
		HttpRequest(const HttpRequest& other);
		HttpRequest& operator=(const HttpRequest& other);
		HttpRequest(HttpRequest&& other) noexcept;
		HttpRequest& operator=(HttpRequest&& other) noexcept;

		const std::string& getMethod() const;
		const std::string& getUri() const;
		const std::string& getHttpVersion() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;

		void setMethod(const std::string& m);
		void setUri(const std::string& u);
		void setHttpVersion(const std::string& v);
		void addHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& b);
};

#endif