#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse
{
	private:
		int _statusCode;
		std::string _statusMessage;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:

		HttpResponse();
		~HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse& operator=(const HttpResponse& other);
		HttpResponse(HttpResponse&& other) noexcept;
		HttpResponse& operator=(HttpResponse&& other) noexcept;

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