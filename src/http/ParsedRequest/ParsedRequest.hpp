#ifndef PARSEDREQUEST_HPP
#define PARSEDREQUEST_HPP

#include <string>
#include <map>
#include <stdexcept>

class ParsedRequest
{
	private:
		std::string _method;
		std::string _uri;
		std::string _httpVersion;
		std::map<std::string, std::string> _headers;
		std::string _body;
		
	public:
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
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;
	
		// Setters
		void setMethod(const std::string& m);
		void setUri(const std::string& u);
		void setHttpVersion(const std::string& v);
		void setBody(const std::string& b);
		
		void addHeader(const std::string& key, const std::string& value);
		void requestReset();
};

#endif