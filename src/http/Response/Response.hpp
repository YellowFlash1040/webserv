#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

class Response
{
	private:
		int _statusCode;
		std::string _statusMessage;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:
		Response();
		~Response();
		Response(const Response& other);
		Response& operator=(const Response& other);
		Response(Response&& other) noexcept;
		Response& operator=(Response&& other) noexcept;

		void responseReset();
		
		int getStatusCode() const;
		const std::string& getStatusMessage() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;

		bool hasHeader(const std::string& key) const;

		void setStatusCode(int code);
		void setStatusMessage(const std::string& message);
		void addHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);

		std::string toString() const;
};

#endif