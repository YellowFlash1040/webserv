#ifndef SERVERRESPONSE_HPP
#define SERVERRESPONSE_HPP

#include <string>
#include <map>

class ServerResponse
{
	private:
		int _statusCode;
		std::string _statusMessage;
		std::map<std::string, std::string> _headers;
		std::string _body;

	public:
		ServerResponse();
		~ServerResponse();
		ServerResponse(const ServerResponse& other);
		ServerResponse& operator=(const ServerResponse& other);
		ServerResponse(ServerResponse&& other) noexcept;
		ServerResponse& operator=(ServerResponse&& other) noexcept;

		void ServerResponseReset();
		
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