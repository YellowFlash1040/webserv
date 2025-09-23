#ifndef CLIENTREQUEST_HPP
#define CLIENTREQUEST_HPP

#include <string>
#include <map>
#include <stdexcept>

class ClientRequest
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
		ClientRequest();
		~ClientRequest();
		ClientRequest(const ClientRequest& other);
		ClientRequest& operator=(const ClientRequest& other);
		ClientRequest(ClientRequest&& other) noexcept;
		ClientRequest& operator=(ClientRequest&& other) noexcept;

		void requestReset();
		
		const std::string& getMethod() const;
		const std::string& getUri() const;
		const std::string& getHttpVersion() const;
		std::string getHeader(const std::string& name) const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;

		void setMethod(const std::string& m);
		void setUri(const std::string& u);
		void setHttpVersion(const std::string& v);
		void addHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& b);
};

#endif